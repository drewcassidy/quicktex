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

#include <gif.h>
#include <string>

#include <cstdint>
#include <memory>

#include "../BlockView.h"
#include "../Color.h"
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

//        error = new_error;
    }
    error = new_error;
}

template <size_t S> void PrepSingleColorTable(MatchList &match_table, MatchList &match_table_half, Interpolator &interpolator) {
    unsigned size = 1 << S;

    std::vector<uint8_t> frame(size * size * 4, 0);
    auto fileName = "lut" + std::to_string(S) + ".gif";
    GifWriter g;
    GifBegin(&g, fileName.c_str(), size, size, 10);

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
                frame[(low + (size * high))*4] = error;
                frame[(low + (size * high))*4+1] = error;
                frame[(low + (size * high))*4+2] = error;
                frame[(low + (size * high))*4+3] = 255;

            }
        }
        GifWriteFrame(&g, frame.data(), size, size, 10);
    }

    GifEnd(&g);
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

    if (pixels.IsSingleColor() || true) {  // for now assume (wrongly) everything is a single-color block
        // single-color pixel block, do it the fast way
        EncodeBlockSingleColor(pixels.Get(0, 0), dest);
        return;
    }

    Color min, max, avg;
    pixels.GetMinMaxAvgRGB(min, max, avg);
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
                mask = 0x55;  // 1111 (min value only, max is ignored)
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

}  // namespace rgbcx