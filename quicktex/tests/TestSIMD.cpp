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

#include "TestSIMD.h"

#include <array>
#include <cassert>
#include <cstdint>
#include <numeric>

#include <xsimd/xsimd.hpp>
#include "../VecUtil.h"

namespace quicktex::tests {

void test_widening_hadd() {
    const auto vec_size = xsimd::batch<int16_t>::size;
    std::array<int16_t, vec_size> buffer;

    std::iota(buffer.begin(), buffer.end(), 1);
    auto v = xsimd::load_unaligned(&buffer[0]);
    auto sum = simd::whadd(v);
    assert(sum == vec_size / 2 * (vec_size + 1));  // Gauss formula

    buffer.fill(1);
    v = xsimd::load_unaligned(&buffer[0]);
    sum = simd::whadd(v);
    assert(sum == vec_size);

    buffer.fill(0);
    v = xsimd::load_unaligned(&buffer[0]);
    sum = simd::whadd(v);
    assert(sum == 0);

    buffer.fill(std::numeric_limits<int16_t>::max());
    v = xsimd::load_unaligned(&buffer[0]);
    sum = simd::whadd(v);
    assert(sum == std::numeric_limits<int16_t>::max() * (int)vec_size);

    buffer.fill(std::numeric_limits<int16_t>::min());
    v = xsimd::load_unaligned(&buffer[0]);
    sum = simd::whadd(v);
    assert(sum == std::numeric_limits<int16_t>::min() * (int)vec_size);

}
}  // namespace quicktex::tests