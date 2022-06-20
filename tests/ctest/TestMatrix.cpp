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

#include <Matrix.h>
#include <gtest/gtest.h>
#include <util/math.h>

#include <array>
#include <cstdlib>

namespace quicktex::tests {

template <typename V> inline void expect_matrix_eq(V value, V expected) {
    if constexpr (std::is_floating_point_v<typename V::value_type>) {
        EXPECT_FLOAT_EQ((value - expected).sum(), 0);
    } else {
        EXPECT_EQ(value, expected);
    }
}

// region Vec_float unit tests
TEST(Vec_float, add) {
    auto a = Vec<float, 3>{1.0f, 1.5f, 2.0f};
    auto b = Vec<float, 3>{2.0f, -2.5f, 3.0f};

    expect_matrix_eq(a + b, {3.0f, -1.0f, 5.0f});
}

TEST(Vec_float, sub) {
    auto a = Vec<float, 3>{1.0f, 1.5f, 2.0f};
    auto b = Vec<float, 3>{3.0f, 1.5f, 1.0f};

    expect_matrix_eq(a - b, {-2.0f, 0.0f, 1.0f});
}

TEST(Vec_float, mul) {
    auto a = Vec<float, 3>{1.0f, 1.5f, 2.0f};
    auto b = Vec<float, 3>{3.0f, 1.5f, 0.0f};

    expect_matrix_eq(a * b, {3.0f, 2.25f, 0.0f});
}

TEST(Vec_float, div) {
    auto a = Vec<float, 3>{1.0f, 1.5f, 2.0f};
    auto b = Vec<float, 3>{2.0f, 1.5f, 1.0f};

    expect_matrix_eq(a / b, {0.5f, 1.0f, 2.0f});
}

TEST(Vec_float, vsum) {
    auto a = Vec<float, 5>{1.0f, 2.0f, 3.5f, 4.0f, -4.0f};

    EXPECT_FLOAT_EQ(a.vsum(), 6.5f);
    EXPECT_FLOAT_EQ(a.sum(), 6.5f);  // sum == vsum for a column vector
}

TEST(Vec_float, dot) {
    auto a = Vec<float, 3>{1.0f, 1.5f, 2.0f};
    auto b = Vec<float, 3>{2.0f, 1.5f, 2.0f};

    EXPECT_FLOAT_EQ(a.dot(b), 8.25f);
}

TEST(Vec_float, abs) {
    auto a = Vec<float, 3>{1.0f, -5.0f, -1.0f};
    auto expected = Vec<float, 3>{1.0f, 5.0f, 1.0f};

    expect_matrix_eq(a.abs(), expected);
}

TEST(Vec_float, clamp) {
    auto a = Vec<float, 6>{-1, -1, -1, 1, 1, 1};
    auto low1 = Vec<float, 6>{-2, -0.5, -2, 0, 2, 0.5};
    auto high1 = Vec<float, 6>{-1.5, 0, 0, 0.5, 3, 2};
    expect_matrix_eq(a.clamp(low1, high1), {-1.5, -0.5, -1, 0.5, 2, 1});

    auto b = Vec<float, 6>{-1, -0.5, 0, 0.2, 0.5, 1};
    expect_matrix_eq(b.clamp(-0.8, 0.3), {-0.8, -0.5, 0, 0.2, 0.3, 0.3});
}
// endregion

// region Vec_int unit tests
TEST(Vec_int, constructor) {
    // scalar constructor
    expect_matrix_eq(Vec<int, 4>(1), {1, 1, 1, 1});

    // initializer list construtor
    expect_matrix_eq(Vec<int, 4>{2, 3, 4, 5}, {2, 3, 4, 5});

    // range constructor
    std::array<int, 4> arr = {6, 7, 8, 9};
    expect_matrix_eq(Vec<int, 4>(arr), {6, 7, 8, 9});

    // iterator constructor
    expect_matrix_eq(Vec<int, 4>(arr.begin()), {6, 7, 8, 9});
}

TEST(Vec_int, subscript) {
    auto a = Vec<int, 4>{1, 3, 1, 2};

    EXPECT_EQ(a[0], 1);
    EXPECT_EQ(a[1], 3);
    EXPECT_EQ(a[2], 1);
    EXPECT_EQ(a[3], 2);

    a[2] = 4;
    EXPECT_EQ(a[2], 4);
}

TEST(Vec_int, getters) {
    auto a = Vec<int, 4>{4, 20, 6, 9};

    for (unsigned i = 0; i < a.size(); i++) {
        EXPECT_EQ(a.get_row(i), a[i]);  // the ith row of a column vector is a scalar
        EXPECT_EQ(a.element(i, 0), a[i]);
        EXPECT_EQ(a.element(i), a[i]);
    }

    EXPECT_EQ(a.get_column(0), a);  // the 0th column of a column-vector is itself
}

TEST(Vec_int, copy) {
    std::array<int, 4> arr{1, 3, 1, 2};
    Vec<int, 4> a(arr);

    expect_matrix_eq(a, {1, 3, 1, 2});

    std::array<int, 4> out{-1, -3, -1, -2};
    std::copy(a.begin(), a.end(), out.begin());

    EXPECT_EQ(out, arr);
}
// endregion
}  // namespace quicktex::tests