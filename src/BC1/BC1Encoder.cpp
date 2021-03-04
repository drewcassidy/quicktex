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

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <memory>
#include <type_traits>

#include "../BlockView.h"
#include "../Color.h"
#include "../Matrix4x4.h"
#include "../Vector4.h"
#include "../Vector4Int.h"
#include "../bitwiseEnums.h"
#include "../util.h"
#include "Histogram.h"
#include "OrderTable.h"
#include "SingleColorTable.h"

namespace rgbcx {
using namespace BC1;

using ColorMode = BC1Encoder::ColorMode;

// constructors

BC1Encoder::BC1Encoder(InterpolatorPtr interpolator) : _interpolator(interpolator) {
    _flags = Flags::None;
    _error_mode = ErrorMode::Check2;
    _endpoint_mode = EndpointMode::PCA;
    _search_rounds = 0;
    _orderings3 = 1;
    _orderings4 = 1;

    OrderTable<3>::Generate();
    OrderTable<4>::Generate();

    assert(OrderTable<3>::generated);
    assert(OrderTable<4>::generated);
}

BC1Encoder::BC1Encoder(unsigned int level, bool allow_3color, bool allow_3color_black) : BC1Encoder(Interpolator::MakeInterpolator()) {
    SetLevel(level, allow_3color, allow_3color_black);
}

BC1Encoder::BC1Encoder(InterpolatorPtr interpolator, unsigned level, bool allow_3color, bool allow_3color_black) : BC1Encoder(interpolator) {
    SetLevel(level, allow_3color, allow_3color_black);
}

BC1Encoder::BC1Encoder(InterpolatorPtr interpolator, Flags flags, ErrorMode error_mode, EndpointMode endpoint_mode, unsigned search_rounds, unsigned orderings4,
                       unsigned orderings3)
    : BC1Encoder(interpolator) {
    SetFlags(flags);
    SetErrorMode(error_mode);
    SetEndpointMode(endpoint_mode);
    SetSearchRounds(search_rounds);
    SetOrderings(orderings4, orderings3);
}

// Getters and Setters
void BC1Encoder::SetLevel(unsigned level, bool allow_3color, bool allow_3color_black) {
    _flags = Flags::None;
    _error_mode = ErrorMode::Check2;
    _endpoint_mode = EndpointMode::PCA;
    _search_rounds = 0;
    _orderings3 = 1;
    _orderings4 = 1;

    switch (level) {
        case 0:
            // Faster/higher quality than stb_dxt default.
            _endpoint_mode = EndpointMode::BoundingBoxInt;
            break;
        case 1:
            // Faster/higher quality than stb_dxt default. a bit higher average quality vs. mode 0.
            _endpoint_mode = EndpointMode::LeastSquares;
            break;
        case 2:
            // On average mode 2 is a little weaker than modes 0/1, but it's stronger on outliers (very tough textures).
            // Slightly stronger than stb_dxt.
            // Uses default settings.
            break;
        case 3:
            // Slightly stronger than stb_dxt HIGHQUAL.
            _flags = Flags::TwoLeastSquaresPasses;
            break;
        case 4:
            _flags = Flags::TwoLeastSquaresPasses | Flags::Use6PowerIters;
            _error_mode = ErrorMode::Full;
            break;
        default:
        case 5:
            // stb_dxt HIGHQUAL + permit 3 color (if it's enabled).
            _flags = Flags::TwoLeastSquaresPasses;
            _error_mode = ErrorMode::Faster;
            break;
        case 6:
            _flags = Flags::TwoLeastSquaresPasses | Flags::UseLikelyTotalOrderings;
            _error_mode = ErrorMode::Faster;
            break;
        case 7:
            _flags = Flags::TwoLeastSquaresPasses | Flags::UseLikelyTotalOrderings;
            _error_mode = ErrorMode::Faster;
            _orderings4 = 4;
            break;
        case 8:
            _flags = Flags::TwoLeastSquaresPasses | Flags::UseLikelyTotalOrderings;
            _error_mode = ErrorMode::Faster;
            _orderings4 = 8;
            break;
        case 9:
            _flags = Flags::TwoLeastSquaresPasses | Flags::UseLikelyTotalOrderings;
            _error_mode = ErrorMode::Check2;
            _orderings4 = 11;
            _orderings3 = 3;
            break;
        case 10:
            _flags = Flags::TwoLeastSquaresPasses | Flags::UseLikelyTotalOrderings;
            _error_mode = ErrorMode::Check2;
            _orderings4 = 20;
            _orderings3 = 8;
            break;
        case 11:
            _flags = Flags::TwoLeastSquaresPasses | Flags::UseLikelyTotalOrderings;
            _error_mode = ErrorMode::Check2;
            _orderings4 = 28;
            _orderings3 = 16;
            break;
        case 12:
            _flags = Flags::TwoLeastSquaresPasses | Flags::UseLikelyTotalOrderings;
            _error_mode = ErrorMode::Check2;
            _orderings4 = 32;
            _orderings3 = 32;
            break;
        case 13:
            _flags = Flags::TwoLeastSquaresPasses | Flags::UseLikelyTotalOrderings | Flags::Use6PowerIters | Flags::TryAllInitialEndpoints;
            _error_mode = ErrorMode::Full;
            _orderings4 = 32;
            _orderings3 = 32;
            _search_rounds = 20;
            break;
        case 14:
            _flags = Flags::TwoLeastSquaresPasses | Flags::UseLikelyTotalOrderings | Flags::Use6PowerIters | Flags::TryAllInitialEndpoints;
            _error_mode = ErrorMode::Full;
            _orderings4 = 32;
            _orderings3 = 32;
            _search_rounds = 32;
            break;
        case 15:
            _flags = Flags::TwoLeastSquaresPasses | Flags::UseLikelyTotalOrderings | Flags::Use6PowerIters | Flags::TryAllInitialEndpoints;
            _error_mode = ErrorMode::Full;
            _orderings4 = 56;
            _orderings3 = 32;
            _search_rounds = 32;
            break;
        case 16:
            _flags = Flags::TwoLeastSquaresPasses | Flags::UseLikelyTotalOrderings | Flags::Use6PowerIters | Flags::TryAllInitialEndpoints;
            _error_mode = ErrorMode::Full;
            _orderings4 = 80;
            _orderings3 = 32;
            _search_rounds = 256;
            break;
        case 17:
            _flags = Flags::TwoLeastSquaresPasses | Flags::UseLikelyTotalOrderings | Flags::Use6PowerIters | Flags::TryAllInitialEndpoints;
            _error_mode = ErrorMode::Full;
            _orderings4 = 128;
            _orderings3 = 32;
            _search_rounds = 256;
            break;
        case 18:
            _flags = Flags::TwoLeastSquaresPasses | Flags::UseLikelyTotalOrderings | Flags::Use6PowerIters | Flags::TryAllInitialEndpoints | Flags::Iterative;
            _error_mode = ErrorMode::Full;
            _orderings4 = 128;
            _orderings3 = 32;
            _search_rounds = 256;
            break;
        case 19:
            // This hidden mode is *extremely* slow and abuses the encoder. It's just for testing/training.
            _flags = Flags::TwoLeastSquaresPasses | Flags::UseLikelyTotalOrderings | Flags::Use6PowerIters | Flags::TryAllInitialEndpoints | Flags::Iterative |
                     Flags::Exhaustive;
            _error_mode = ErrorMode::Full;
            _orderings4 = 128;
            _orderings3 = 32;
            _search_rounds = 256;
            break;
    }

    if (level >= 5 && allow_3color) { _flags |= Flags::Use3ColorBlocks; }
    if (level >= 5 && allow_3color_black) { _flags |= Flags::Use3ColorBlocksForBlackPixels; }

    _orderings4 = clamp(_orderings4, 1U, OrderTable<4>::BestOrderCount);
    _orderings3 = clamp(_orderings3, 1U, OrderTable<3>::BestOrderCount);
}

void BC1Encoder::SetOrderings(unsigned orderings4, unsigned orderings3) {
    _orderings4 = clamp(orderings4, 1U, OrderTable<4>::BestOrderCount);
    _orderings3 = clamp(orderings3, 1U, OrderTable<3>::BestOrderCount);
}

// Public methods
void BC1Encoder::EncodeBlock(Color4x4 pixels, BC1Block *dest) const {
    if (pixels.IsSingleColor()) {
        // single-color pixel block, do it the fast way
        WriteBlockSolid(pixels.Get(0, 0), dest);
        return;
    }

    auto metrics = pixels.GetMetrics();

    bool needs_block_error = (_flags & Flags::UseLikelyTotalOrderings | Flags::Use3ColorBlocks) != Flags::None;
    needs_block_error |= (_error_mode != ErrorMode::None);
    needs_block_error |= (_search_rounds > 0);
    needs_block_error |= metrics.has_black && ((_flags & Flags::Use3ColorBlocksForBlackPixels) != Flags::None);
    ErrorMode error_mode = needs_block_error ? _error_mode : ErrorMode::None;

    assert(!((_error_mode == ErrorMode::None) && needs_block_error));

    const unsigned total_ls_passes = (_flags & Flags::TwoLeastSquaresPasses) != Flags::None ? 2 : 1;
    const unsigned total_ep_rounds = needs_block_error && ((_flags & Flags::TryAllInitialEndpoints) != Flags::None) ? 2 : 1;
    const unsigned total_cf_iters = (_flags & Flags::Iterative) != Flags::None ? 2 : 1;

    // Initial block generation
    EncodeResults orig;
    EncodeResults result;
    for (unsigned round = 0; round < total_ep_rounds; round++) {
        EndpointMode endpoint_mode = (round == 1) ? EndpointMode::BoundingBox : _endpoint_mode;

        EncodeResults trial_orig;
        FindEndpoints(pixels, trial_orig, metrics, endpoint_mode);

        EncodeResults trial_result = trial_orig;

        FindSelectors<ColorMode::FourColor>(pixels, trial_result, error_mode);
        RefineBlockLS<ColorMode::FourColor>(pixels, trial_result, metrics, error_mode, total_ls_passes);

        if (!needs_block_error || trial_result.error < result.error) {
            result = trial_result;
            orig = trial_orig;
        }
    }

    // First refinement pass using ordered cluster fit
    if (result.error > 0 && (bool)(_flags & Flags::UseLikelyTotalOrderings)) {
        for (unsigned iter = 0; iter < total_cf_iters; iter++) { RefineBlockCF<ColorMode::FourColor>(pixels, result, metrics, _error_mode, _orderings4); }
    }

    // try for 3-color block
    if (result.error > 0 && (bool)(_flags & Flags::Use3ColorBlocks)) {
        EncodeResults trial_result = orig;

        FindSelectors<ColorMode::ThreeColor>(pixels, trial_result, ErrorMode::Full);
        RefineBlockLS<ColorMode::ThreeColor>(pixels, trial_result, metrics, ErrorMode::Full, total_ls_passes);

        // First refinement pass using ordered cluster fit
        if (trial_result.error > 0 && (bool)(_flags & Flags::UseLikelyTotalOrderings)) {
            for (unsigned iter = 0; iter < total_cf_iters; iter++) {
                RefineBlockCF<ColorMode::ThreeColor>(pixels, trial_result, metrics, ErrorMode::Full, _orderings3);
            }
        }

        if (trial_result.error < result.error) { result = trial_result; }
    }

    // try for 3-color block with black
    if (result.error > 0 && (bool)(_flags & Flags::Use3ColorBlocksForBlackPixels) && metrics.has_black && !metrics.max.IsBlack()) {
        EncodeResults trial_result;
        BlockMetrics metrics_no_black = pixels.GetMetrics(true);

        FindEndpoints(pixels, trial_result, metrics_no_black, EndpointMode::PCA, true);
        FindSelectors<ColorMode::ThreeColorBlack>(pixels, trial_result, ErrorMode::Full);
        RefineBlockLS<ColorMode::ThreeColorBlack>(pixels, trial_result, metrics_no_black, ErrorMode::Full, total_ls_passes);

        if (trial_result.error < result.error) { result = trial_result; }
    }

    // refine endpoints by searching for nearby colors
    if (result.error > 0 && _search_rounds > 0) { EndpointSearch(pixels, result); }

    WriteBlock(result, dest);
}

// Private methods
void BC1Encoder::WriteBlockSolid(Color color, BC1Block *dest) const {
    uint8_t mask = 0xAA;  // 2222
    uint16_t min16, max16;

    if ((color.r | color.g | color.b) == 0) {
        // quick shortcut for all-black blocks
        min16 = 0;
        max16 = 1;
        mask = 0x55;  // 1111 (Min value only, max is ignored)
    } else {
        // why is there no subscript operator for shared_ptr<array>
        EncodeResults result;
        FindEndpointsSingleColor(result, color, false);

        if ((_flags & (Flags::Use3ColorBlocks | Flags::Use3ColorBlocksForBlackPixels)) != Flags::None) {
            EncodeResults result_3color;
            FindEndpointsSingleColor(result_3color, color, true);

            if (result_3color.error < result.error) { result = result_3color; }
        }

        min16 = result.low.Pack565Unscaled();
        max16 = result.high.Pack565Unscaled();

        if (result.color_mode == ColorMode::Solid) {
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
        } else if (max16 > min16) {
            std::swap(min16, max16);  // assure 3-color blocks
        }
    }

    dest->SetLowColor(max16);
    dest->SetHighColor(min16);
    dest->selectors[0] = mask;
    dest->selectors[1] = mask;
    dest->selectors[2] = mask;
    dest->selectors[3] = mask;
}

void BC1Encoder::WriteBlock(EncodeResults &block, BC1Block *dest) const {
    BC1Block::UnpackedSelectors selectors;
    uint16_t color1 = block.low.Pack565Unscaled();
    uint16_t color0 = block.high.Pack565Unscaled();
    std::array<uint8_t, 4> lut;

    assert(block.color_mode != ColorMode::Incomplete);

    if ((bool)(block.color_mode & ColorMode::FourColor)) {
        lut = {1, 3, 2, 0};

        if (color1 > color0) {
            std::swap(color1, color0);
            lut = {0, 2, 3, 1};
        } else if (color1 == color0) {
            if (color1 > 0) {
                color1--;
                lut = {0, 0, 0, 0};
            } else {
                assert(color1 == 0 && color0 == 0);
                color0 = 1;
                color1 = 0;
                lut = {1, 1, 1, 1};
            }
        }

        assert(color0 > color1);
    } else {
        lut = {1, 2, 0, 3};

        if (color1 < color0) {
            std::swap(color1, color0);
            lut = {0, 2, 1, 3};
        }

        assert(color0 <= color1);
    }

    for (unsigned i = 0; i < 16; i++) {
        unsigned x = i % 4;
        unsigned y = i / 4;
        selectors[y][x] = lut[block.selectors[i]];
        if (block.color_mode == ColorMode::ThreeColor) { assert(selectors[y][x] != 3); }
    }

    dest->SetLowColor(color0);
    dest->SetHighColor(color1);
    dest->PackSelectors(selectors);
}

void BC1Encoder::FindEndpointsSingleColor(EncodeResults &block, Color color, bool is_3color) const {
    auto &match5 = is_3color ? _single_match5_half : _single_match5;
    auto &match6 = is_3color ? _single_match6_half : _single_match6;

    BC1MatchEntry match_r = match5->at(color.r);
    BC1MatchEntry match_g = match6->at(color.g);
    BC1MatchEntry match_b = match5->at(color.b);

    block.color_mode = is_3color ? ColorMode::ThreeColorSolid : ColorMode::Solid;
    block.error = match_r.error + match_g.error + match_b.error;
    block.low = Color(match_r.low, match_g.low, match_b.low);
    block.high = Color(match_r.high, match_g.high, match_b.high);
    // selectors decided when writing, no point deciding them now
}

void BC1Encoder::FindEndpointsSingleColor(EncodeResults &block, Color4x4 &pixels, Color color, bool is_3color) const {
    std::array<Color, 4> colors = _interpolator->InterpolateBC1(block.low, block.high, is_3color);
    Vector4Int result_vector = (Vector4Int)colors[2];

    FindEndpointsSingleColor(block, color, is_3color);

    block.error = 0;
    for (unsigned i = 0; i < 16; i++) {
        Vector4Int pixel_vector = (Vector4Int)pixels.Get(i);
        auto diff = pixel_vector - result_vector;
        block.error += diff.SqrMag();
        block.selectors[i] = 1;
    }
}

void BC1Encoder::FindEndpoints(Color4x4 pixels, EncodeResults &block, const BlockMetrics &metrics, EndpointMode endpoint_mode, bool ignore_black) const {
    if (metrics.is_greyscale) {
        // specialized greyscale case
        const unsigned fr = pixels.Get(0).r;

        if (metrics.max.r - metrics.min.r < 2) {
            // single color block
            uint8_t fr5 = (uint8_t)scale8To5(fr);
            uint8_t fr6 = (uint8_t)scale8To6(fr);

            block.low = Color(fr5, fr6, fr5);
            block.high = block.low;
        } else {
            uint8_t lr5 = scale8To5(metrics.min.r);
            uint8_t lr6 = scale8To6(metrics.min.r);

            uint8_t hr5 = scale8To5(metrics.max.r);
            uint8_t hr6 = scale8To6(metrics.max.r);

            block.low = Color(lr5, lr6, lr5);
            block.high = Color(hr5, hr6, hr5);
        }
    } else if (endpoint_mode == EndpointMode::LeastSquares) {
        //  2D Least Squares approach from Humus's example, with added inset and optimal rounding.
        Color diff = Color(metrics.max.r - metrics.min.r, metrics.max.g - metrics.min.g, metrics.max.b - metrics.min.b);
        Vector4 l = {0, 0, 0};
        Vector4 h = {0, 0, 0};

        auto &sums = metrics.sums;
        auto &min = metrics.min;

        unsigned chan0 = (unsigned)diff.MaxChannelRGB();  // primary axis of the bounding box
        l[chan0] = (float)min[chan0];
        h[chan0] = (float)min[chan0];

        assert((diff[chan0] >= diff[(chan0 + 1) % 3]) && (diff[chan0] >= diff[(chan0 + 2) % 3]));

        std::array<unsigned, 3> sums_xy;

        for (unsigned i = 0; i < 16; i++) {
            auto val = pixels.Get(i);
            for (unsigned c = 0; c < 3; c++) { sums_xy[c] += val[chan0] * val[c]; }
        }

        const unsigned sum_x = (unsigned)sums[chan0];
        const unsigned sum_xx = sums_xy[chan0];

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
                const unsigned sum_y = (unsigned)sums[chan];
                const unsigned sum_xy = sums_xy[chan];

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

        block.low = Color::PreciseRound565(l);
        block.high = Color::PreciseRound565(h);
    } else if (endpoint_mode == EndpointMode::BoundingBox) {
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

        block.low = Color::PreciseRound565(l);
        block.high = Color::PreciseRound565(h);
    } else if (endpoint_mode == EndpointMode::BoundingBoxInt) {
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

        block.low = min.ScaleTo565();
        block.high = max.ScaleTo565();
    } else if (endpoint_mode == EndpointMode::PCA) {
        // the slow way
        // Select 2 colors along the principle axis. (There must be a faster/simpler way.)
        auto min = Vector4::FromColorRGB(metrics.min);
        auto max = Vector4::FromColorRGB(metrics.max);
        auto avg = Vector4::FromColorRGB(metrics.avg);

        Vector4 axis = {306, 601, 117};  // Luma vector
        Matrix4x4 covariance = Matrix4x4::Identity();
        const unsigned total_power_iters = (_flags & Flags::Use6PowerIters) != Flags::None ? 6 : 4;

        for (unsigned i = 0; i < 16; i++) {
            auto val = pixels.Get(i);
            if (ignore_black && val.IsBlack()) continue;

            auto color_vec = Vector4::FromColorRGB(val);
            Vector4 diff = color_vec - avg;
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
            auto val = pixels.Get(i);
            if (ignore_black && val.IsBlack()) continue;

            auto color_vec = Vector4::FromColorRGB(val);
            // since axis is constant here, I dont think its magnitude actually matters,
            // since we only care about the min or max dot product
            float dot = color_vec.Dot(axis);
            if (dot > max_dot) {
                max_dot = dot;
                max_index = i;
            }
            if (dot < min_dot) {
                min_dot = dot;
                min_index = i;
            }
        }

        block.low = pixels.Get(min_index).ScaleTo565();
        block.high = pixels.Get(max_index).ScaleTo565();
    }

    block.color_mode = ColorMode::Incomplete;
}

template <ColorMode M> void BC1Encoder::FindSelectors(Color4x4 &pixels, EncodeResults &block, ErrorMode error_mode) const {
    assert(!((error_mode != ErrorMode::Full) && (bool)(M & ColorMode::ThreeColor)));
    assert(!(bool)(M & ColorMode::Solid));

    const int color_count = (unsigned)M & 0x0F;

    std::array<Color, 4> colors = _interpolator->InterpolateBC1(block.low, block.high, color_count == 3);
    std::array<Vector4Int, 4> color_vectors;

    if (color_count == 4) {
        color_vectors = {Vector4Int::FromColorRGB(colors[0]), Vector4Int::FromColorRGB(colors[2]), Vector4Int::FromColorRGB(colors[3]),
                         Vector4Int::FromColorRGB(colors[1])};
    } else {
        color_vectors = {Vector4Int::FromColorRGB(colors[0]), Vector4Int::FromColorRGB(colors[2]), Vector4Int::FromColorRGB(colors[1]),
                         Vector4Int::FromColorRGB(colors[3])};
    }

    unsigned total_error = 0;

    if (error_mode == ErrorMode::None || error_mode == ErrorMode::Faster) {
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

            if (error_mode == ErrorMode::Faster) {
                // llvm is just going to unswitch this anyways so its not an issue
                auto diff = pixel_vector - color_vectors[selector];
                total_error += diff.SqrMag();
                if (i % 4 != 0 && total_error >= block.error) break;  // check only once per row if we're generating too much error
            }

            block.selectors[i] = selector;
        }
    } else if (error_mode == ErrorMode::Check2) {
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
    } else if (error_mode == ErrorMode::Full) {
        unsigned max_sel = (bool)(M == ColorMode::ThreeColor) ? 3 : 4;

        for (unsigned i = 0; i < 16; i++) {
            unsigned best_error = UINT_MAX;
            uint8_t best_sel = 0;
            Vector4Int pixel_vector = Vector4Int::FromColorRGB(pixels.Get(i));

            // exhasustively check every pixel's distance from each color, and calculate the error
            for (uint8_t j = 0; j < max_sel; j++) {
                auto diff = color_vectors[j] - pixel_vector;
                unsigned err = diff.SqrMag();
                if (err < best_error || ((err == best_error) && (j == 3))) {
                    best_error = err;
                    best_sel = j;
                }
            }

            total_error += best_error;
            if (total_error >= block.error) { break; }

            assert(best_sel < max_sel);
            block.selectors[i] = best_sel;
        }
    } else {
        assert(false);
    }
    block.error = total_error;
    block.color_mode = M;
}

template <ColorMode M> bool BC1Encoder::RefineEndpointsLS(Color4x4 pixels, EncodeResults &block, BlockMetrics metrics) const {
    const int color_count = (unsigned)M & 0x0F;
    static_assert(color_count == 3 || color_count == 4);
    static_assert(!(bool)(M & ColorMode::Solid));
    assert(block.color_mode != ColorMode::Incomplete);

    int denominator = color_count - 1;

    Vector4 q00 = {0, 0, 0};
    Vector4 matrix = Vector4(0);

    for (unsigned i = 0; i < 16; i++) {
        const Color color = pixels.Get(i);
        const uint8_t sel = block.selectors[i];

        if ((bool)(M & ColorMode::ThreeColorBlack) && color.IsBlack()) continue;
        if ((bool)(M & ColorMode::ThreeColor) && sel == 3U) continue;  // NOTE: selectors for 3-color are in linear order here, but not in original
        assert(sel < color_count);

        const Vector4Int color_vector = Vector4Int::FromColorRGB(color);
        q00 += color_vector * sel;

        matrix += OrderTable<color_count>::Weights[sel];
    }

    // invert matrix
    float det = matrix.Determinant2x2();  // z00 * z11 - z01 * z10;
    if (fabs(det) < 1e-8f) {
        block.color_mode = ColorMode::Incomplete;
        return false;
    }

    std::swap(matrix[0], matrix[3]);
    matrix *= Vector4(1, -1, -1, 1);
    matrix *= ((float)denominator / 255.0f) / det;

    Vector4 q10 = (metrics.sums * denominator) - q00;

    Vector4 low = (matrix[0] * q00) + (matrix[1] * q10);
    Vector4 high = (matrix[2] * q00) + (matrix[3] * q10);

    block.color_mode = M;
    block.low = Color::PreciseRound565(low);
    block.high = Color::PreciseRound565(high);
    return true;
}

template <ColorMode M> void BC1Encoder::RefineEndpointsLS(std::array<Vector4, 17> &sums, EncodeResults &block, Vector4 &matrix, Hash hash) const {
    const int color_count = (unsigned)M & 0x0F;
    static_assert(color_count == 3 || color_count == 4);
    static_assert(!(bool)(M & ColorMode::Solid));
    assert(block.color_mode != ColorMode::Incomplete);

    int denominator = color_count - 1;

    Vector4 q10 = {0, 0, 0};
    unsigned level = 0;
    Histogram<color_count> h = OrderTable<color_count>::Orders[hash];
    for (unsigned i = 0; i < (color_count - 1); i++) {
        level += h[i];
        q10 += sums[level];
    }

    Vector4 q00 = (sums[16] * (float)denominator) - q10;

    Vector4 low = (matrix[0] * q00) + (matrix[1] * q10);
    Vector4 high = (matrix[2] * q00) + (matrix[3] * q10);

    block.color_mode = M;
    block.low = Color::PreciseRound565(low);
    block.high = Color::PreciseRound565(high);
}

template <ColorMode M>
void BC1Encoder::RefineBlockLS(Color4x4 &pixels, EncodeResults &block, BlockMetrics &metrics, ErrorMode error_mode, unsigned passes) const {
    assert(error_mode != ErrorMode::None || passes == 1);

    for (unsigned pass = 0; pass < passes; pass++) {
        EncodeResults trial_result = block;
        Vector4 low, high;

        bool multicolor = RefineEndpointsLS<ColorMode::FourColor>(pixels, trial_result, metrics);
        if (!multicolor) {
            FindEndpointsSingleColor(trial_result, pixels, metrics.avg, (M != ColorMode::FourColor));
        } else {
            FindSelectors<M>(pixels, trial_result, error_mode);
        }

        if (trial_result.low == block.low && trial_result.high == block.high) break;

        if (error_mode == ErrorMode::None || trial_result.error < block.error) {
            block = trial_result;
        } else {
            return;
        }
    }
}

template <ColorMode M>
void BC1Encoder::RefineBlockCF(Color4x4 &pixels, EncodeResults &block, BlockMetrics &metrics, ErrorMode error_mode, unsigned orderings) const {
    const int color_count = (unsigned)M & 0x0F;
    static_assert(color_count == 3 || color_count == 4);
    static_assert(!(bool)(M & ColorMode::Solid));
    assert(block.color_mode != ColorMode::Incomplete);

    using OrderTable = OrderTable<color_count>;
    using Hist = Histogram<color_count>;

    EncodeResults orig = block;
    Hist h = Hist(orig.selectors);

    Hash start_hash = OrderTable::GetHash(h);

    Vector4 axis = orig.high.ScaleFrom565() - orig.low.ScaleFrom565();
    std::array<Vector4, 16> color_vectors;
    std::array<uint32_t, 16> dots;

    for (unsigned i = 0; i < 16; i++) {
        color_vectors[i] = Vector4::FromColorRGB(pixels.Get(i));
        int dot = 0x1000000 + (int)color_vectors[i].Dot(axis);
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

    const unsigned q_total = ((_flags & Flags::Exhaustive) != Flags::None) ? OrderTable::OrderCount : orderings;
    for (Hash q = 0; q < q_total; q++) {
        Hash trial_hash = ((_flags & Flags::Exhaustive) != Flags::None) ? q : OrderTable::BestOrders[start_hash][q];
        Vector4 trial_matrix = OrderTable::GetFactors(trial_hash);

        EncodeResults trial_result = orig;
        Vector4 low, high;
        if (OrderTable::IsSingleColor(trial_hash)) {
            FindEndpointsSingleColor(trial_result, pixels, metrics.avg, (color_count == 3));
        } else {
            RefineEndpointsLS<M>(sums, trial_result, trial_matrix, trial_hash);
            FindSelectors<M>(pixels, trial_result, error_mode);
        }

        if (trial_result.error < block.error) { block = trial_result; }
        if (trial_result.error == 0) break;
    }
}

void BC1Encoder::EndpointSearch(Color4x4 &pixels, EncodeResults &block) const {
    if ((bool)(block.color_mode & ColorMode::Solid)) return;

    static const std::array<Vector4Int, 16> Voxels = {{
        {1, 0, 0, 3},    // 0
        {0, 1, 0, 4},    // 1
        {0, 0, 1, 5},    // 2
        {-1, 0, 0, 0},   // 3
        {0, -1, 0, 1},   // 4
        {0, 0, -1, 2},   // 5
        {1, 1, 0, 9},    // 6
        {1, 0, 1, 10},   // 7
        {0, 1, 1, 11},   // 8
        {-1, -1, 0, 6},  // 9
        {-1, 0, -1, 7},  // 10
        {0, -1, -1, 8},  // 11
        {-1, 1, 0, 13},  // 12
        {1, -1, 0, 12},  // 13
        {0, -1, 1, 15},  // 14
        {0, 1, -1, 14},  // 15
    }};

    unsigned prev_improvement_index = 0;
    int forbidden_direction = -1;

    for (unsigned i = 0; i < _search_rounds; i++) {
        const unsigned voxel_index = (unsigned)(i & 15);
        assert((unsigned)Voxels[(unsigned)Voxels[voxel_index][3]][3] == voxel_index);  // make sure voxels are symmetrical

        if ((int)(i & 31) == forbidden_direction) continue;

        Vector4Int delta = Voxels[voxel_index];
        EncodeResults trial_result = block;

        if (i & 16) {
            trial_result.low.r = (uint8_t)clamp(trial_result.low.r + delta[0], 0, 31);
            trial_result.low.g = (uint8_t)clamp(trial_result.low.g + delta[1], 0, 63);
            trial_result.low.b = (uint8_t)clamp(trial_result.low.b + delta[2], 0, 31);
        } else {
            trial_result.high.r = (uint8_t)clamp(trial_result.high.r + delta[0], 0, 31);
            trial_result.high.g = (uint8_t)clamp(trial_result.high.g + delta[1], 0, 63);
            trial_result.high.b = (uint8_t)clamp(trial_result.high.b + delta[2], 0, 31);
        }

        switch (block.color_mode) {
            default:
            case ColorMode::FourColor:
                FindSelectors<ColorMode::FourColor>(pixels, trial_result, _error_mode);
                break;
            case ColorMode::ThreeColor:
                FindSelectors<ColorMode::ThreeColor>(pixels, trial_result, ErrorMode::Full);
                break;
            case ColorMode::ThreeColorBlack:
                FindSelectors<ColorMode::ThreeColorBlack>(pixels, trial_result, ErrorMode::Full);
                break;
        }

        if (trial_result.error < block.error) {
            block = trial_result;

            forbidden_direction = delta[3] | (int)(i & 16);
            prev_improvement_index = i;
        }

        if (i - prev_improvement_index > 32) break;
    }
}

}  // namespace rgbcx