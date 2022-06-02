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
#include "Vec.h"
#include "bitbash.h"

namespace quicktex::color {

constexpr size_t uint5_max = (1 << 5) - 1;
constexpr size_t uint6_max = (1 << 6) - 1;

template <size_t N> struct MidpointTable {
   public:
    constexpr MidpointTable() : _values() {
        constexpr float fN = (float)N;
        for (unsigned i = 0; i < N - 1; i++) { _values[i] = ((float)i / fN) + (0.5f / fN); }
        _values[N - 1] = 1e+37f;
    }

    float operator[](size_t i) const {
        assert(i < N);
        return _values[i];
    }

   private:
    float _values[N];
};

constexpr MidpointTable<32> Midpoints5bit;
constexpr MidpointTable<64> Midpoints6bit;

template <typename T> Vec<T, 3> scale_to_565(Vec<T, 3> unscaled) {
    return Vec<T, 3>{scale_from_8<T, 5>(unscaled.r()), scale_from_8<T, 6>(unscaled.g()),
                     scale_from_8<T, 5>(unscaled.b())};
}

template <typename T> Vec<T, 3> scale_from_565(Vec<T, 3> scaled) {
    return Vec<T, 3>{scale_to_8<T, 5>(scaled.r()), scale_to_8<T, 6>(scaled.g()), scale_to_8<T, 5>(scaled.b())};
}

template <typename T = int16_t> Vec<T, 3> precise_round_565(Vec<float, 3> &v) {
    auto scaled = v * Vec<float, 3>{uint5_max, uint6_max, uint5_max};       // rescale by from (0,1) to (0,int_max)
    auto rounded = (Vec<T, 3>)scaled;                                       // downcast to integral type
    rounded = rounded.clamp({0, 0, 0}, {uint5_max, uint6_max, uint5_max});  // clamp to avoid out of bounds float errors

    // increment each channel if above the rounding point
    if (v.r() > Midpoints5bit[rounded.r()]) rounded.r()++;
    if (v.g() > Midpoints6bit[rounded.g()]) rounded.g()++;
    if (v.b() > Midpoints5bit[rounded.b()]) rounded.b()++;

    assert(rounded.r() <= uint5_max);
    assert(rounded.g() <= uint6_max);
    assert(rounded.b() <= uint5_max);

    return rounded;
}
}  // namespace quicktex::color