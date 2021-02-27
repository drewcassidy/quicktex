/*  Python-rgbcx Texture Compression Library
    Copyright (C) 2021 Andrew Cassidy <drewcassidy@me.com>
    Partially derived from rgbcx.h written by Richard Geldreich <richgel99@gmail.com>
    and licenced under the public domain

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "BC1Encoder.h"

#include <array>
#include <cassert>
#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>

#include "../BlockView.h"
#include "../Color.h"
#include "../Interpolator.h"
#include "../Matrix4x4.h"
#include "../Vector4.h"
#include "../Vector4Int.h"
#include "../bitwiseEnums.h"
#include "../util.h"
#include "OrderTable.h"
#include "SingleColorTable.h"

namespace rgbcx {
using InterpolatorPtr = std::shared_ptr<Interpolator>;
using Hist3 = OrderTable<3>::Histogram;
using Hist4 = OrderTable<4>::Histogram;

// region Free Functions/Templates
// endregion

// Static Fields
OrderTable<3> *BC1Encoder::order_table3 = nullptr;
OrderTable<4> *BC1Encoder::order_table4 = nullptr;
std::mutex BC1Encoder::order_table_mutex = std::mutex();
bool BC1Encoder::order_tables_generated = false;

BC1Encoder::BC1Encoder(InterpolatorPtr interpolator) : _interpolator(interpolator) {
    _flags = Flags::UseFasterMSEEval | Flags::TwoLeastSquaresPasses;

    // generate lookup tables
    order_table_mutex.lock();
    if (!order_tables_generated) {
        assert(order_table3 == nullptr);
        assert(order_table4 == nullptr);

        order_table3 = new OrderTable<3>();
        order_table4 = new OrderTable<4>();
        order_tables_generated = true;
    }
    assert(order_table3 != nullptr);
    assert(order_table4 != nullptr);
    order_table_mutex.unlock();
}

void BC1Encoder::EncodeBlock(Color4x4 pixels, BC1Block *dest) const {
    auto r_view = pixels.GetChannel(0);
    auto g_view = pixels.GetChannel(1);
    auto b_view = pixels.GetChannel(2);

    Color first = pixels.Get(0, 0);

    if (pixels.IsSingleColor()) {
        // single-color pixel block, do it the fast way
        EncodeBlockSingleColor(first, dest);
        return;
    }

    auto metrics = pixels.GetMetrics();

    bool needs_block_error = (_flags & Flags::UseLikelyTotalOrderings | Flags::Use3ColorBlocks | Flags::UseFullMSEEval) != Flags::None;
    needs_block_error |= (_search_rounds > 0);
    needs_block_error |= metrics.has_black && ((_flags & Flags::Use3ColorBlocksForBlackPixels) != Flags::None);

    unsigned total_ls_passes = (_flags & Flags::TwoLeastSquaresPasses) != Flags::None ? 2 : 1;
    unsigned total_ep_rounds = needs_block_error && ((_flags & Flags::TryAllInitialEndpoints) != Flags::None) ? 2 : 1;

    // Initial block generation
    EncodeResults result;
    for (unsigned round = 0; round < total_ep_rounds; round++) {
        Flags modified_flags = _flags;
        if (round == 1) {
            modified_flags &= ~(Flags::Use2DLS | Flags::BoundingBoxInt);
            modified_flags |= Flags::BoundingBox;
        }

        EncodeResults round_result;
        FindEndpoints(pixels, modified_flags, metrics, round_result.low, round_result.high);
        FindSelectors4(pixels, round_result, needs_block_error);

        for (unsigned pass = 0; pass < total_ls_passes; pass++) {
            EncodeResults trial_result = round_result;
            Vector4 low, high;

            bool multicolor = ComputeEndpointsLS(pixels, trial_result, metrics, false, false);

            if (trial_result.low == round_result.low && trial_result.high == round_result.high) break;

            FindSelectors4(pixels, trial_result, needs_block_error);

            if (!needs_block_error || trial_result.error < round_result.error) {
                round_result = trial_result;
            } else {
                break;
            }
        }
        if (!needs_block_error || round_result.error < result.error) { result = round_result; }
    }

    // First refinement pass using ordered cluster fit
    if (result.error > 0 && (_flags & Flags::UseLikelyTotalOrderings) != Flags::None) {
        const unsigned total_iters = (_flags & Flags::Iterative) != Flags::None ? 2 : 1;
        for (unsigned iter = 0; iter < total_iters; iter++) {
            EncodeResults orig = result;
            Hist4 h(orig.selectors);

            const Hash order_index = order_table4->GetHash(h);

            Color low = orig.low.ScaleFrom565();
            Color high = orig.high.ScaleFrom565();

            Vector4Int axis = high - low;
            std::array<Vector4, 16> color_vectors;

            std::array<uint32_t, 16> dots;
            for (unsigned i = 0; i < 16; i++) {
                color_vectors[i] = Vector4::FromColorRGB(pixels.Get(i));
                int dot = 0x1000000 + color_vectors[i].Dot(axis);
                assert(dot >= 0);
                dots[i] = (uint32_t)(dot << 4) | i;
            }

            std::sort(dots.begin(), dots.end());

            // we now have a list of indices and their dot products along the primary axis
            std::array<Vector4, 17> sums;
            for (unsigned i = 0; i < 16; i++) {
                const unsigned p = dots[i] & 0xF;
                sums[i + 1] = sums[i] + color_vectors[p];
            }

            const unsigned q_total = ((_flags & Flags::Exhaustive) != Flags::None) ? order_table4->UniqueOrderings
                                                                                   : (unsigned)clampi(_orderings4, MIN_TOTAL_ORDERINGS, MAX_TOTAL_ORDERINGS4);
            for (unsigned q = 0; q < q_total; q++) {
                Hash s = ((_flags & Flags::Exhaustive) != Flags::None) ? q : g_best_total_orderings4[order_index][q];

                EncodeResults trial = orig;
                Vector4 low, high;
                if (order_table4->IsSingleColor(order_index)) {
                    trial.is_1_color = true;
                    trial.is_3_color = false;
                } else {
                }
            }
        }
    }

    if (result.low == result.high) {
        EncodeBlockSingleColor(metrics.avg, dest);
    } else {
        EncodeBlock4Color(result, dest);
    }
}

void BC1Encoder::EncodeBlockSingleColor(Color color, BC1Block *dest) const {
    uint8_t mask = 0xAA;  // 2222
    uint16_t min16, max16;

    bool using_3color = false;

    // why is there no subscript operator for shared_ptr<array>

    auto match_r = _single_match5[color.r];
    auto match_g = _single_match6[color.g];
    auto match_b = _single_match5[color.b];

    if ((_flags & (Flags::Use3ColorBlocks | Flags::Use3ColorBlocksForBlackPixels)) != Flags::None) {
        auto match_r_half = _single_match5_half[color.r];
        auto match_g_half = _single_match6_half[color.g];
        auto match_b_half = _single_match5_half[color.b];

        const unsigned err4 = match_r.error + match_g.error + match_b.error;
        const unsigned err3 = match_r_half.error + match_g_half.error + match_b_half.error;

        if (err3 < err4) {
            min16 = Color::Pack565Unscaled(match_r_half.low, match_g_half.low, match_b_half.low);
            max16 = Color::Pack565Unscaled(match_r_half.high, match_g_half.high, match_b_half.high);

            if (max16 > min16) std::swap(min16, max16);
            using_3color = true;
        }
    }

    if (!using_3color) {
        min16 = Color::Pack565Unscaled(match_r.low, match_g.low, match_b.low);
        max16 = Color::Pack565Unscaled(match_r.high, match_g.high, match_b.high);

        if (min16 == max16) {
            // make sure this isnt accidentally a 3-color block
            // so make max16 > min16 (l > h)
            if (min16 > 0) {
                min16--;
                mask = 0;  // endpoints are equal so mask doesnt matter
            } else {
                assert(min16 == 0 && max16 == 0);
                max16 = 1;
                min16 = 0;
                mask = 0x55;  // 1111 (Min value only, max is ignored)
            }
        } else if (max16 < min16) {
            std::swap(min16, max16);
            mask = 0xFF;  // invert mask to 3333
        }
        assert(max16 > min16);
    }

    dest->SetLowColor(max16);
    dest->SetHighColor(min16);
    dest->selectors[0] = mask;
    dest->selectors[1] = mask;
    dest->selectors[2] = mask;
    dest->selectors[3] = mask;
}

void BC1Encoder::EncodeBlock4Color(EncodeResults &block, BC1Block *dest) const {
    const std::array<uint8_t, 4> lut = {0, 2, 3, 1};
    if (block.low == block.high) {
        EncodeBlockSingleColor(block.low.ScaleFrom565() /* Color(255, 0, 255)*/, dest);
        return;
    }

    uint8_t mask = 0;
    uint16_t low = block.low.Pack565Unscaled();
    uint16_t high = block.high.Pack565Unscaled();
    if (low < high) {
        std::swap(low, high);
        mask = 0x55;
    }

    BC1Block::UnpackedSelectors selectors;

    for (unsigned i = 0; i < 16; i++) {
        unsigned x = i % 4;
        unsigned y = i / 4;
        selectors[y][x] = lut[block.selectors[i]];
    }

    assert(low > high);
    dest->SetLowColor(low);
    dest->SetHighColor(high);
    dest->PackSelectors(selectors, mask);
}

void BC1Encoder::FindEndpoints(Color4x4 pixels, BC1Encoder::Flags flags, const BC1Encoder::BlockMetrics metrics, Color &low, Color &high) const {
    if (metrics.is_greyscale) {
        // specialized greyscale case
        const unsigned fr = pixels.Get(0).r;

        if (metrics.max.r - metrics.min.r < 2) {
            // single color block
            low.r = high.r = (uint8_t)scale8To5(fr);
            low.g = high.g = (uint8_t)scale8To6(fr);
            low.b = high.b = low.r;
        } else {
            low.r = low.b = scale8To5(metrics.min.r);
            low.g = scale8To6(metrics.min.r);

            high.r = high.b = scale8To5(metrics.max.r);
            high.g = scale8To6(metrics.max.r);
        }
    } else if ((flags & Flags::Use2DLS) != Flags::None) {
        //  2D Least Squares approach from Humus's example, with added inset and optimal rounding.
        Color diff = Color(metrics.max.r - metrics.min.r, metrics.max.g - metrics.min.g, metrics.max.b - metrics.min.b);
        Vector4 l = {0, 0, 0};
        Vector4 h = {0, 0, 0};

        auto &sums = metrics.sums;
        auto &min = metrics.min;
        auto &max = metrics.max;

        unsigned chan0 = (unsigned)diff.MaxChannelRGB();  // primary axis of the bounding box
        l[chan0] = (float)min[chan0];
        h[chan0] = (float)min[chan0];

        assert((diff[chan0] >= diff[(chan0 + 1) % 3]) && (diff[chan0] >= diff[(chan0 + 2) % 3]));

        std::array<unsigned, 3> sums_xy;

        for (unsigned i = 0; i < 16; i++) {
            auto val = pixels.Get(i);
            for (unsigned c = 0; c < 3; c++) { sums_xy[c] += val[chan0] * val[c]; }
        }

        const auto &sum_x = sums[chan0];
        const auto &sum_xx = sums_xy[chan0];

        float denominator = (float)(16 * sum_xx) - (float)(sum_x * sum_x);

        // once per secondary axis, calculate high and low using least squares
        if (fabs(denominator) > 1e-8f) {
            for (unsigned i = 1; i < 3; i++) {
                /* each secondary axis is fitted with a linear formula of the form
                 *  y = ax + b
                 * where y is the secondary axis and x is the primary axis
                 *  a = (m∑xy - ∑x∑y) / m∑x² - (∑x)²
                 *  b = (∑x²∑y - ∑xy∑x) / m∑x² - (∑x)²
                 * see Giordano/Weir pg.103 */
                const unsigned chan = (chan0 + i) % 3;
                const unsigned &sum_y = sums[chan];
                const unsigned &sum_xy = sums_xy[chan];

                float a = (float)((16 * sum_xy) - (sum_x * sum_y)) / denominator;
                float b = (float)((sum_xx * sum_y) - (sum_xy * sum_x)) / denominator;

                l[chan] = b + (a * l[chan0]);
                h[chan] = b + (a * h[chan0]);
            }
        }

        // once per axis, inset towards the center by 1/16 of the delta and scale
        for (unsigned c = 0; c < 3; c++) {
            float inset = (h[c] - l[c]) / 16.0f;

            l[c] = ((l[c] + inset) / 255.0f);
            h[c] = ((h[c] - inset) / 255.0f);
        }

        low = Color::PreciseRound565(l);
        high = Color::PreciseRound565(h);
    } else if ((flags & Flags::BoundingBox) != Flags::None) {
        // Algorithm from icbc.h compress_dxt1_fast()
        Vector4 l, h;
        const float bias = 8.0f / 255.0f;

        // rescale and inset values
        for (unsigned c = 0; c < 3; c++) {  // heh, c++
            l[c] = (float)metrics.min[c] / 255.0f;
            h[c] = (float)metrics.max[c] / 255.0f;

            float inset = (h[c] - l[c] - bias) / 16.0f;
            l[c] += inset;
            h[c] -= inset;
        }

        // Select the correct diagonal across the bounding box
        int icov_xz = 0, icov_yz = 0;
        for (unsigned i = 0; i < 16; i++) {
            int b = (int)pixels.Get(i).b - metrics.avg.b;
            icov_xz += b * (int)pixels.Get(i).r - metrics.avg.r;
            icov_yz += b * (int)pixels.Get(i).g - metrics.avg.g;
        }

        if (icov_xz < 0) std::swap(l[0], h[0]);
        if (icov_yz < 0) std::swap(l[1], h[1]);

        low = Color::PreciseRound565(l);
        high = Color::PreciseRound565(h);
    } else if ((flags & Flags::BoundingBoxInt) != Flags::None) {
        // Algorithm from icbc.h compress_dxt1_fast(), but converted to integer.

        Color min, max;

        // rescale and inset values
        for (unsigned c = 0; c < 3; c++) {
            int inset = ((int)(metrics.max[c] - metrics.min[c]) - 8) >> 4;  // 1/16 of delta, with bias

            min[c] = clamp255(metrics.min[c] + inset);
            max[c] = clamp255(metrics.max[c] - inset);
        }

        int icov_xz = 0, icov_yz = 0;
        for (unsigned i = 0; i < 16; i++) {
            int b = (int)pixels.Get(i).b - metrics.avg.b;
            icov_xz += b * (int)pixels.Get(i).r - metrics.avg.r;
            icov_yz += b * (int)pixels.Get(i).g - metrics.avg.g;
        }

        if (icov_xz < 0) std::swap(min.r, max.r);
        if (icov_yz < 0) std::swap(min.g, max.g);

        low = min.ScaleTo565();
        high = max.ScaleTo565();
    } else {
        // the slow way
        // Select 2 colors along the principle axis. (There must be a faster/simpler way.)
        auto min = Vector4::FromColorRGB(metrics.min);
        auto max = Vector4::FromColorRGB(metrics.max);
        auto avg = Vector4::FromColorRGB(metrics.avg);

        std::array<Vector4, 16> colors;

        Vector4 axis = {306, 601, 117};  // Luma vector
        Matrix4x4 covariance = Matrix4x4::Identity();
        const unsigned total_power_iters = (flags & Flags::Use6PowerIters) != Flags::None ? 6 : 4;

        for (unsigned i = 0; i < 16; i++) {
            colors[i] = Vector4::FromColorRGB(pixels.Get(i));
            Vector4 diff = colors[i] - avg;
            for (unsigned c1 = 0; c1 < 3; c1++) {
                for (unsigned c2 = c1; c2 < 3; c2++) {
                    covariance[c1][c2] += (diff[c1] * diff[c2]);
                    assert(c1 <= c2);
                }
            }
        }

        covariance /= 255.0f;
        covariance.Mirror();

        Vector4 delta = max - min;

        // realign r and g axes to match
        if (covariance[0][2] < 0) delta[0] = -delta[0];  // r vs b
        if (covariance[1][2] < 0) delta[1] = -delta[1];  // g vs b

        // using the covariance matrix, stretch the delta vector towards the primary axis of the data using power iteration
        // the end result of this may actually be the same as the least squares approach, will have to do more research
        for (unsigned power_iter = 0; power_iter < total_power_iters; power_iter++) { delta = covariance * delta; }

        // if we found any correlation, then this is our new axis. otherwise we fallback to the luma vector
        float k = delta.MaxAbs(3);
        if (k >= 2) { axis = delta * (2048.0f / k); }

        axis *= 16;

        float min_dot = INFINITY;
        float max_dot = -INFINITY;

        unsigned min_index = 0, max_index = 0;

        for (unsigned i = 0; i < 16; i++) {
            // since axis is constant here, I dont think its magnitude actually matters,
            // since we only care about the min or max dot product
            float dot = colors[i].Dot(axis);
            if (dot > max_dot) {
                max_dot = dot;
                max_index = i;
            }
            if (dot < min_dot) {
                min_dot = dot;
                min_index = i;
            }
        }

        low = pixels.Get(min_index).ScaleTo565();
        high = pixels.Get(max_index).ScaleTo565();
    }
}

unsigned BC1Encoder::FindSelectors4(Color4x4 pixels, BC1Encoder::EncodeResults &block, bool use_err) const {
    // colors in selector order, 0, 1, 2, 3
    // 0 = low color, 1 = high color, 2/3 = interpolated
    std::array<Color, 4> colors = _interpolator->InterpolateBC1(block.low, block.high, false);
    std::array<Vector4Int, 4> color_vectors = {(Vector4Int)colors[0], (Vector4Int)colors[2], (Vector4Int)colors[3], (Vector4Int)colors[1]};
    unsigned total_error = 0;

    if (!use_err || (_flags & Flags::UseFasterMSEEval) != Flags::None) {
        Vector4Int axis = color_vectors[3] - color_vectors[0];
        std::array<int, 4> dots;
        for (unsigned i = 0; i < 4; i++) { dots[i] = axis.Dot(color_vectors[i]); }
        int t0 = dots[0] + dots[1], t1 = dots[1] + dots[2], t2 = dots[2] + dots[3];
        axis *= 2;

        for (unsigned i = 0; i < 16; i++) {
            Vector4Int pixel_vector = (Vector4Int)pixels.Get(i);
            int dot = axis.Dot(pixel_vector);
            uint8_t level = (dot <= t0) + (dot < t1) + (dot < t2);
            uint8_t selector = 3 - level;
            assert(level < 4);
            assert(selector < 4);

            if ((_flags & Flags::UseFasterMSEEval) != Flags::None) {
                // llvm is just going to unswitch this anyways so its not an issue
                auto diff = pixel_vector - color_vectors[selector];
                total_error += diff.SqrMag();
                if (i % 4 != 0 && total_error >= block.error) break;  // check only once per row if we're generating too much error
            }

            block.selectors[i] = selector;
        }
    } else if ((_flags & Flags::UseFullMSEEval) != Flags::None) {
        for (unsigned i = 0; i < 16; i++) {
            unsigned best_error = UINT_MAX;
            uint8_t best_sel = 0;
            Vector4Int pixel_vector = (Vector4Int)pixels.Get(i);

            // exhasustively check every pixel's distance from each color, and calculate the error
            for (uint8_t j = 0; j < 4; j++) {
                auto diff = color_vectors[j] - pixel_vector;
                unsigned err = diff.SqrMag();
                if (err < best_error || ((err == best_error) && (j == 3))) {
                    best_error = err;
                    best_sel = j;
                }
            }

            total_error += best_error;
            if (total_error >= block.error) break;

            block.selectors[i] = best_sel;
        }
    } else {
        Vector4Int axis = color_vectors[3] - color_vectors[0];
        const float f = 4.0f / ((float)axis.SqrMag() + .00000125f);

        for (unsigned i = 0; i < 16; i++) {
            Vector4Int pixel_vector = (Vector4Int)pixels.Get(i);
            auto diff = pixel_vector - color_vectors[0];
            float sel_f = (float)diff.Dot(axis) * f + 0.5f;
            uint8_t sel = (uint8_t)clampi((int)sel_f, 1, 3);

            unsigned err0 = (color_vectors[sel - 1] - pixel_vector).SqrMag();
            unsigned err1 = (color_vectors[sel] - pixel_vector).SqrMag();

            uint8_t best_sel = sel;
            unsigned best_err = err1;
            if (err0 == err1) {
                // prefer non-interpolation
                if ((best_sel) == 1) best_sel = 0;
            } else if (err0 < best_err) {
                best_sel = sel - 1;
                best_err = err0;
            }

            total_error += best_err;

            if (total_error >= block.error) break;

            block.selectors[i] = best_sel;
        }
    }
    block.is_3_color = false;
    block.is_1_color = false;
    block.error = total_error;
    return total_error;
}

bool BC1Encoder::ComputeEndpointsLS(Color4x4 pixels, EncodeResults &block, BlockMetrics metrics, bool is_3color, bool use_black) const {
    Vector4 low, high;
    Vector4 q00 = {0, 0, 0};
    unsigned weight_accum = 0;
    for (unsigned i = 0; i < 16; i++) {
        const Color color = pixels.Get(i);
        const int sel = (int)block.selectors[i];

        if (use_black && color.IsBlack()) continue;
        if (is_3color && sel == 3) continue;  // NOTE: selectors for 3-color are in linear order here, but not in original
        assert(sel <= 3);

        const Vector4Int color_vector = Vector4Int::FromColorRGB(color);
        q00 += color_vector * sel;
        weight_accum += g_weight_vals4[sel];
    }

    int denominator = is_3color ? 2 : 3;
    Vector4 q10 = (metrics.sums * denominator) - q00;

    float z00 = (float)((weight_accum >> 16) & 0xFF);
    float z10 = (float)((weight_accum >> 8) & 0xFF);
    float z11 = (float)(weight_accum & 0xFF);
    float z01 = z10;

    // invert matrix
    float det = z00 * z11 - z01 * z10;
    if (fabs(det) < 1e-8f) {
        block.is_1_color = true;
        return false;
    }

    det = ((float)denominator / 255.0f) / det;

    float iz00, iz01, iz10, iz11;
    iz00 = z11 * det;
    iz01 = -z01 * det;
    iz10 = -z10 * det;
    iz11 = z00 * det;

    low = (q00 * iz00) + (q10 * iz01);
    high = (q00 * iz10) + (q10 * iz11);

    block.is_1_color = false;
    block.low = Color::PreciseRound565(low);
    block.high = Color::PreciseRound565(high);
    return true;
}
/*
bool BC1Encoder::ComputeEndpointsLS(Color4x4 pixels, EncodeResults &block, BlockMetrics metrics, Hash hash, Vector4 &matrix, std::array<Vector4, 17> &sums,
                                    bool is_3color, bool use_black) const {
    unsigned f1, f2, f3;
    int denominator = is_3color ? 2 : 3;

    if (is_3color) {
        order_table3->GetUniqueOrderingSums(hash, f1, f2, f3);
    } else {
        order_table4->GetUniqueOrderingSums(hash, f1, f2, f3);
    }
}*/

}  // namespace rgbcx