/*  Quicktex Texture Compression Library
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

#pragma once

#include <array>
#include <cstdint>
#include <memory>

#include "../../util.h"
#include "../interpolator/Interpolator.h"

namespace quicktex::s3tc  {

struct BC1MatchEntry {
    uint8_t high;
    uint8_t low;
    uint8_t error;
};

using MatchList = std::array<BC1MatchEntry, 256>;
using MatchListPtr = std::shared_ptr<MatchList>;
using InterpolatorPtr = std::shared_ptr<Interpolator>;

/**
 * Lookup table for single-color blocks
 * @tparam B Number of bits (5 or 6)
 * @tparam N Number of colors (3 or 4)
 */
template <size_t B, size_t N> MatchListPtr SingleColorTable(InterpolatorPtr interpolator) {
    constexpr size_t Size = 1 << B;
    MatchListPtr matches = std::make_shared<MatchList>();

    static_assert((B == 5 && Size == 32) || (B == 6 && Size == 64));
    static_assert(N == 4 || N == 3);

    bool ideal = interpolator->IsIdeal();
    bool use_8bit = interpolator->CanInterpolate8Bit();

    for (unsigned i = 0; i < 256; i++) {
        unsigned error = 256;

        // TODO: Can probably avoid testing for values that definitely wont yield good results,
        // e.g. low8 and high8 both much smaller or larger than index
        for (uint8_t low = 0; low < Size; low++) {
            uint8_t low8 = (B == 5) ? scale5To8(low) : scale6To8(low);

            for (uint8_t high = 0; high < Size; high++) {
                uint8_t high8 = (B == 5) ? scale5To8(high) : scale6To8(high);
                uint8_t value;

                if (use_8bit) {
                    value = interpolator->Interpolate8(high8, low8);
                } else {
                    value = (B == 5) ? interpolator->Interpolate5(high, low) : interpolator->Interpolate6(high, low);
                }

                unsigned new_error = iabs(value - (int)i);

                // We only need to factor in 3% error in BC1 ideal mode.
                if (ideal) new_error += (iabs(high8 - (int)low8) * 3) / 100;

                if ((new_error < error) || (new_error == error && low == high)) {
                    assert(new_error <= UINT8_MAX);

                    (*matches)[i].low = (uint8_t)low;
                    (*matches)[i].high = (uint8_t)high;
                    (*matches)[i].error = (uint8_t)new_error;

                    error = new_error;
                }
            }
        }
    }
    return matches;
}
}  // namespace quicktex::s3tc