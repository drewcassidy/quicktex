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

#include <utest.h>

#include <array>
#include <cassert>
#include <cstdint>
#include <numeric>
#include <type_traits>
#include <xsimd/xsimd.hpp>

#include "../VecUtil.h"

namespace quicktex::tests {

template <typename T> constexpr auto make_arrays() {
    std::vector<std::array<T, xsimd::batch<T>::size>> arrays;
    std::array<T, xsimd::batch<T>::size> buffer;

    std::iota(buffer.begin(), buffer.end(), 1);
    arrays.push_back(buffer);

    buffer.fill(1);
    arrays.push_back(buffer);

    buffer.fill(0);
    arrays.push_back(buffer);

    buffer.fill(std::numeric_limits<T>::max());
    arrays.push_back(buffer);

    if (std::is_signed_v<T>) {
        std::iota(buffer.begin(), buffer.end(), -1);
        arrays.push_back(buffer);

        buffer.fill(-1);
        arrays.push_back(buffer);

        buffer.fill(std::numeric_limits<T>::min());
        arrays.push_back(buffer);
    }

    return arrays;
}

template <typename T, size_t N> long int sumArray(std::array<T, N> const &arg) {
    return std::accumulate(arg.begin(), arg.end(), (long)0);
}

#define UTEST_WHADD(TYPE)                            \
    UTEST(simd, whadd_##TYPE) {                      \
        for (auto arr : make_arrays<TYPE>()) {       \
            auto v = xsimd::load_unaligned(&arr[0]); \
            auto vsum = simd::whadd(v);              \
            auto ssum = sumArray(arr);               \
            ASSERT_EQ(vsum, ssum);                   \
        }                                            \
    }

UTEST_WHADD(int8_t);
UTEST_WHADD(uint8_t);
UTEST_WHADD(int16_t);
UTEST_WHADD(uint16_t);
UTEST_WHADD(int32_t);
UTEST_WHADD(uint32_t);

}  // namespace quicktex::tests