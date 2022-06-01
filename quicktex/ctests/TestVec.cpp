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

#include <stdlib.h>  // for abs
#include <utest.h>   // for UTEST

#include <array>  // for operator==

#include "../Vec.h"   // for Vec, ope...
#include "../util.h"  // for abs

namespace quicktex::tests {

// region Vec_float unit tests
UTEST(Vec_float, add) {
    auto a = Vec<float, 3>{1.0f, 1.5f, 2.0f};
    auto b = Vec<float, 3>{2.0f, -2.5f, 3.0f};
    auto expected = Vec<float, 3>{3.0f, -1.0f, 5.0f};
    float diff = ((a + b) - expected).sqr_mag();

    ASSERT_LT(diff, 0.01f);
}

UTEST(Vec_float, sub) {
    auto a = Vec<float, 3>{1.0f, 1.5f, 2.0f};
    auto b = Vec<float, 3>{3.0f, 1.5f, 1.0f};
    auto expected = Vec<float, 3>{-2.0f, 0.0f, 1.0f};
    float diff = ((a - b) - expected).sqr_mag();

    ASSERT_LT(diff, 0.01f);
}

UTEST(Vec_float, mul) {
    auto a = Vec<float, 3>{1.0f, 1.5f, 2.0f};
    auto b = Vec<float, 3>{3.0f, 1.5f, 0.0f};
    auto expected = Vec<float, 3>{3.0f, 2.25f, 0.0f};
    float diff = ((a * b) - expected).sqr_mag();

    ASSERT_LT(diff, 0.01f);
}

UTEST(Vec_float, div) {
    auto a = Vec<float, 3>{1.0f, 1.5f, 2.0f};
    auto b = Vec<float, 3>{2.0f, 1.5f, 1.0f};
    auto expected = Vec<float, 3>{0.5f, 1.0f, 2.0f};
    float diff = ((a / b) - expected).sqr_mag();

    ASSERT_LT(diff, 0.01f);
}

UTEST(Vec_float, sum) {
    auto a = Vec<float, 5>{1.0f, 2.0f, 3.5f, 4.0f, -4.0f};
    auto diff = abs(a.sum() - 6.5f);

    ASSERT_LT(diff, 0.01f);
}

UTEST(Vec_float, dot) {
    auto a = Vec<float, 3>{1.0f, 1.5f, 2.0f};
    auto b = Vec<float, 3>{2.0f, 1.5f, 2.0f};
    auto diff = abs(a.dot(b) - 8.25f);

    ASSERT_LT(diff, 0.01f);
}

UTEST(Vec_float, abs) {
    auto a = Vec<float, 3>{1.0f, -5.0f, -1.0f};
    auto expected = Vec<float, 3>{1.0f, 5.0f, 1.0f};
    auto diff = (a.abs() - expected).sqr_mag();

    ASSERT_LT(diff, 0.01f);
}

UTEST(Vec_float, clamp) {
    auto a = Vec<float, 6>{-1, -1, -1, 1, 1, 1};
    auto low1 = Vec<float, 6>{-2, -0.5, -2, 0, 2, 0.5};
    auto high1 = Vec<float, 6>{-1.5, 0, 0, 0.5, 3, 2};
    auto result1 = a.clamp(low1, high1);
    auto expected1 = Vec<float, 6>{-1.5, -0.5, -1, 0.5, 2, 1};
    auto diff1 = (result1 - expected1).sqr_mag();

    ASSERT_LT(diff1, 0.01f);

    auto b = Vec<float, 6>{-1, -0.5, 0, 0.2, 0.5, 1};
    auto result2 = b.clamp(-0.8, 0.3);
    auto expected2 = Vec<float, 6>{-0.8, -0.5, 0, 0.2, 0.3, 0.3};
    auto diff2 = (result2 - expected2).sqr_mag();

    ASSERT_LT(diff2, 0.01f);
}

// endregion

// region Vec_int unit tests
UTEST(Vec_int, subscript) {
    auto a = Vec<int, 4>{1, 3, 1, 2};

    ASSERT_EQ(a[0], 1);
    ASSERT_EQ(a[1], 3);
    ASSERT_EQ(a[2], 1);
    ASSERT_EQ(a[3], 2);

    a[2] = 4;
    ASSERT_EQ(a[2], 4);
}

UTEST(Vec_int, copy) {
    std::array<int, 4> arr{1, 3, 1, 2};
    Vec<int, 4> a(arr);
    Vec<int, 4> expected{1, 3, 1, 2};

    ASSERT_TRUE(a == expected);

    std::array<int, 4> out{-1, -3, -1, -2};
    a.copy(out.begin());

    ASSERT_TRUE(out == arr);
}
// endregion
}  // namespace quicktex::tests