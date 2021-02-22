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
#include <cmath>
#include <cstdint>
#include <memory>

#include "../BlockView.h"
#include "../Color.h"
#include "../Matrix4x4.h"
#include "../Vector4.h"
#include "../bitwiseEnums.h"

namespace rgbcx {
using MatchList = std::array<BC1MatchEntry, 256>;
using MatchListPtr = std::shared_ptr<MatchList>;
using InterpolatorPtr = std::shared_ptr<Interpolator>;

// region Free Functions/Templates
inline void PrepSingleColorTableEntry(unsigned &error, MatchList &match_table, uint8_t v, unsigned i, uint8_t low, uint8_t high, uint8_t low8, uint8_t high8,
                                      bool ideal) {
    unsigned new_error = iabs(v - (int)i);

    // We only need to factor in 3% error in BC1 ideal mode.
    if (ideal) new_error += (iabs(high8 - (int)low8) * 3) / 100;

    // Favor equal endpoints, for lower error on actual GPU's which approximate the interpolation.
    if ((new_error < error) || (new_error == error && low == high)) {
        assert(new_error <= UINT8_MAX);

        match_table[i].low = (uint8_t)low;
        match_table[i].high = (uint8_t)high;
        match_table[i].error = (uint8_t)new_error;

        error = new_error;
    }
}

template <size_t S> void PrepSingleColorTable(MatchList &match_table, MatchList &match_table_half, Interpolator &interpolator) {
    unsigned size = 1 << S;

    assert((S == 5 && size == 32) || (S == 6 && size == 64));

    bool ideal = interpolator.IsIdeal();
    bool use_8bit = interpolator.CanInterpolate8Bit();

    for (unsigned i = 0; i < 256; i++) {
        unsigned error = 256;
        unsigned error_half = 256;

        // TODO: Can probably avoid testing for values that definitely wont yield good results,
        // e.g. low8 and high8 both much smaller or larger than index
        for (uint8_t low = 0; low < size; low++) {
            uint8_t low8 = (S == 5) ? scale5To8(low) : scale6To8(low);

            for (uint8_t high = 0; high < size; high++) {
                uint8_t high8 = (S == 5) ? scale5To8(high) : scale6To8(high);
                uint8_t value, value_half;

                if (use_8bit) {
                    value = interpolator.Interpolate8(high8, low8);
                    value_half = interpolator.InterpolateHalf8(high8, low8);
                } else {
                    value = (S == 5) ? interpolator.Interpolate5(high, low) : interpolator.Interpolate6(high, low);
                    value_half = (S == 5) ? interpolator.InterpolateHalf5(high, low) : interpolator.InterpolateHalf6(high, low);
                }

                PrepSingleColorTableEntry(error, match_table, value, i, low, high, low8, high8, ideal);
                PrepSingleColorTableEntry(error_half, match_table_half, value_half, i, low, high, low8, high8, ideal);
            }
        }
    }
}
// endregion

BC1Encoder::BC1Encoder(InterpolatorPtr interpolator) : _interpolator(interpolator) {
    PrepSingleColorTable<5>(*_single_match5, *_single_match5_half, *_interpolator);
    PrepSingleColorTable<6>(*_single_match6, *_single_match6_half, *_interpolator);
}

void BC1Encoder::EncodeBlock(Color4x4 pixels, BC1Block *dest) const {
    auto r_view = pixels.GetChannel(0);
    auto g_view = pixels.GetChannel(1);
    auto b_view = pixels.GetChannel(2);

    Color first = pixels.Get(0, 0);

    if (pixels.IsSingleColor()) {  // for now assume (wrongly) everything is a single-color block
        // single-color pixel block, do it the fast way
        EncodeBlockSingleColor(first, dest);
        return;
    }

    auto metrics = pixels.GetMetrics();
}

void BC1Encoder::EncodeBlockSingleColor(Color color, BC1Block *dest) const {
    uint8_t mask = 0xAA;  // 2222
    uint16_t min16, max16;

    bool using_3color = false;

    // why is there no subscript operator for shared_ptr<array>
    MatchList &match5 = *_single_match5;
    MatchList &match6 = *_single_match6;
    MatchList &match5_half = *_single_match5_half;
    MatchList &match6_half = *_single_match6_half;

    BC1MatchEntry match_r = match5[color.r];
    BC1MatchEntry match_g = match6[color.g];
    BC1MatchEntry match_b = match5[color.b];

    if ((_flags & (Flags::Use3ColorBlocks | Flags::Use3ColorBlocksForBlackPixels)) != Flags::None) {
        BC1MatchEntry match_r_half = match5_half[color.r];
        BC1MatchEntry match_g_half = match6_half[color.g];
        BC1MatchEntry match_b_half = match5_half[color.b];

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

        assert(diff[chan0] >= diff[(chan0 + 1) % 3] && diff[chan0] >= diff[(chan0 + 2) % 3]);

        std::array<unsigned, 3> sums_xy;

        for (unsigned i = 0; i < 16; i++) {
            auto val = pixels.Get(i);
            for (unsigned c = 0; c < 3; c++) { sums_xy[c] += val[chan0] * val[c]; }
        }

        auto &sum_x = sums[chan0];
        auto &sum_xx = sums_xy[chan0];

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
        Matrix4x4 covariance;
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

        // using the covariance matrix, iteratively stretch the delta vector towards the primary axis of the data
        for (unsigned power_iter = 0; power_iter < total_power_iters; power_iter++) { delta = covariance * delta; }

        // if we found any correlation, then this is our new axis. otherwise we fallback to the luma vector
        float k = delta.MaxAbs(3);
        if (k > 2) { axis = delta * (2048.0f / k); }

        float min_dot = INFINITY;
        float max_dot = -INFINITY;

        unsigned min_index, max_index;

        for (unsigned i = 0; i < 16; i++) {
            // since axis is constant here, I dont think its magnitude actually matters,
            // since we only care about the min or max dot product
            float dot = colors[i].Dot(axis);
            if (dot > max_dot) {
                max_dot = dot;
                max_index = i;
            } else if (dot < min_dot) {
                min_dot = dot;
                min_index = i;
            }
        }

        low = pixels.Get(min_index).ScaleTo565();
        high = pixels.Get(max_index).ScaleTo565();
    }
}
}  // namespace rgbcx