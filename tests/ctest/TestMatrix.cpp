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

#define EXPECT_MATRIX_EQ(value, expected)                                           \
    {                                                                               \
        auto v = value;                                                             \
        auto e = expected;                                                          \
        if constexpr (std::is_floating_point_v<typename decltype(v)::value_type>) { \
            for (unsigned i = 0; i < v.elements; i++) {                             \
                EXPECT_FLOAT_EQ(v.element(i), e.element(i)) << "At index " << i;    \
            }                                                                       \
        } else {                                                                    \
            EXPECT_EQ(v, e);                                                        \
        }                                                                           \
    }

constexpr size_t fibn(size_t n) {
    return (n < 2) ? n : fibn(n - 1) + fibn(n - 2);
}

template <typename T> constexpr T sqr(T n) { return n * n; }

template <typename Op, typename... Args> constexpr void foreach (Op f, Args... args) { (f(args), ...); }

template <typename T> class MatrixTest : public testing::Test {
   public:
    using Scalar = T;
    template <size_t M> using Vec = quicktex::Vec<T, M>;
    template <size_t M, size_t N> using Matrix = quicktex::Matrix<T, M, N>;

    template <typename M> constexpr M iota(T start = 0, T stride = 1) {
        M result(0);
        for (unsigned i = 0; i < M::elements; i++) { result.element(i) = (static_cast<T>(i) + start) * stride; }
        return result;
    }

    template <typename M> constexpr M sqr(T start = 0, T stride = 1) {
        M result(0);
        for (unsigned i = 0; i < M::elements; i++) {
            result.element(i) = static_cast<T>((i + start) * (i + start) * stride);
        }
        return result;
    }

    template <typename M> constexpr M fib(T start = 0) {
        M result(0);
        for (unsigned i = 0; i < M::elements; i++) { result.element(i) = fibn(i + start); }
        return result;
    }

    static constexpr auto sizes = std::make_tuple(Vec<4>(0), Vec<7>(0), Matrix<4, 4>(0), Matrix<5, 6>(0));

    template <typename Op> constexpr void foreach_size(Op f) {
        auto foreach = [f]<typename... Args>(Args... args) { (f(args), ...); };
        std::apply(foreach, sizes);
    }
};

using Scalars = ::testing::Types<uint8_t, int8_t, uint16_t, int16_t, uint32_t, int32_t, float, double>;
TYPED_TEST_SUITE(MatrixTest, Scalars);

#define IOTA(M, start, stride) TestFixture::template iota<M>(start, stride)
#define SQR(M, start, stride) TestFixture::template sqr<M>(start, stride)
#define FIB(M, start) TestFixture::template fib<M>(start)

TYPED_TEST(MatrixTest, negate) {
    if constexpr (std::unsigned_integral<typename TestFixture::Scalar>) {
        GTEST_SKIP();
    } else {
        TestFixture::foreach_size([&]<typename M>(M) {
            EXPECT_MATRIX_EQ(-IOTA(M, 0, 1), IOTA(M, 0, -1));
            EXPECT_MATRIX_EQ(-IOTA(M, 0, -1), IOTA(M, 0, 1));
        });
    }
}

TYPED_TEST(MatrixTest, add) {
    TestFixture::foreach_size([&]<typename M>(M) {
        EXPECT_MATRIX_EQ(IOTA(M, 0, 1) + IOTA(M, 0, 3), IOTA(M, 0, 4));
        EXPECT_MATRIX_EQ(IOTA(M, 0, 2) + IOTA(M, 0, 2), IOTA(M, 0, 4));
        if constexpr (std::unsigned_integral<typename TestFixture::Scalar>) {
            EXPECT_MATRIX_EQ(IOTA(M, 0, 3) + IOTA(M, 0, -1), IOTA(M, 0, 2));
        }
    });
}

TYPED_TEST(MatrixTest, subtract) {
    TestFixture::foreach_size([&]<typename M>(M) {
        EXPECT_MATRIX_EQ(IOTA(M, 0, 4) - IOTA(M, 0, 1), IOTA(M, 0, 3));
        EXPECT_MATRIX_EQ(IOTA(M, 0, 2) - IOTA(M, 0, 2), IOTA(M, 0, 0));
        if constexpr (std::unsigned_integral<typename TestFixture::Scalar>) {
            EXPECT_MATRIX_EQ(IOTA(M, 0, 3) - IOTA(M, 0, -1), IOTA(M, 0, 4));
            EXPECT_MATRIX_EQ(IOTA(M, 0, 1) - IOTA(M, 0, 3), IOTA(M, 0, -2));
        }
    });
}

TYPED_TEST(MatrixTest, multiply) {
    TestFixture::foreach_size([&]<typename M>(M) {
        EXPECT_MATRIX_EQ(IOTA(M, 0, 2) * 2, IOTA(M, 0, 4));
        EXPECT_MATRIX_EQ(IOTA(M, 0, 2) * 0, M(0));

        if constexpr (!std::is_unsigned_v<typename M::value_type>) {
            EXPECT_MATRIX_EQ(IOTA(M, 0, 2) * -2, IOTA(M, 0, -4));
        }

        if constexpr (std::numeric_limits<typename M::value_type>::max() >= sqr(M::elements - 1)) {
            EXPECT_MATRIX_EQ(IOTA(M, 0, 1) * IOTA(M, 0, 1), SQR(M, 0, 1));
        }

        if constexpr (std::numeric_limits<typename M::value_type>::max() >= sqr(M::elements - 1) * 3) {
            EXPECT_MATRIX_EQ(IOTA(M, 0, 1) * IOTA(M, 0, 3), SQR(M, 0, 3));
            EXPECT_MATRIX_EQ(IOTA(M, 0, 0) * IOTA(M, 0, 3), SQR(M, 0, 0));
        }

        if constexpr (std::numeric_limits<typename M::value_type>::max() >= sqr(M::elements - 1) * 4) {
            EXPECT_MATRIX_EQ(IOTA(M, 0, 2) * IOTA(M, 0, 2), SQR(M, 0, 4));
            if constexpr (!std::is_unsigned_v<typename M::value_type>) {
                EXPECT_MATRIX_EQ(IOTA(M, 0, 4) * IOTA(M, 0, -1), SQR(M, 0, -4));
                EXPECT_MATRIX_EQ(IOTA(M, 0, -4) * IOTA(M, 0, -1), SQR(M, 0, 4));
            }
        }
    });
}

TYPED_TEST(MatrixTest, divide) {
    TestFixture::foreach_size([&]<typename M>(M) {
        EXPECT_MATRIX_EQ(IOTA(M, 0, 4) / 2, IOTA(M, 0, 2));
        EXPECT_MATRIX_EQ(IOTA(M, 0, 2) / 1, IOTA(M, 0, 2));

        if constexpr (!std::is_unsigned_v<typename M::value_type>) {
            EXPECT_MATRIX_EQ(IOTA(M, 0, 4) / -2, IOTA(M, 0, -2));
            EXPECT_MATRIX_EQ(IOTA(M, 0, -4) / -2, IOTA(M, 0, 2));
        }

        if constexpr (std::numeric_limits<typename M::value_type>::max() >= sqr(M::elements)) {
            EXPECT_MATRIX_EQ(SQR(M, 1, 1) / IOTA(M, 1, 1), IOTA(M, 1, 1));
        }

        if constexpr (std::numeric_limits<typename M::value_type>::max() >= sqr(M::elements) * 3) {
            EXPECT_MATRIX_EQ(SQR(M, 1, 3) / IOTA(M, 1, 1), IOTA(M, 1, 3));
            EXPECT_MATRIX_EQ(SQR(M, 1, 3) / IOTA(M, 1, 3), IOTA(M, 1, 1));
        }

        if constexpr (std::numeric_limits<typename M::value_type>::max() >= sqr(M::elements) * 4) {
            EXPECT_MATRIX_EQ(SQR(M, 1, 4) / IOTA(M, 1, 2), IOTA(M, 1, 2));
            if constexpr (!std::is_unsigned_v<typename M::value_type>) {
                EXPECT_MATRIX_EQ(SQR(M, 1, -4) / IOTA(M, 1, -1), IOTA(M, 1, 4));
                EXPECT_MATRIX_EQ(SQR(M, 1, 4) / IOTA(M, 1, -1), IOTA(M, 1, -4));
            }
        }
    });
}

TYPED_TEST(MatrixTest, abs) {
    if constexpr (std::unsigned_integral<typename TestFixture::Scalar>) {
        GTEST_SKIP();
    } else {
        TestFixture::foreach_size([&]<typename M>(M) {
            EXPECT_MATRIX_EQ(IOTA(M, 0, -1).abs(), IOTA(M, 0, 1));
            EXPECT_MATRIX_EQ(IOTA(M, 0, 1).abs(), IOTA(M, 0, 1));
        });
    }
}

TYPED_TEST(MatrixTest, clamp) {
    TestFixture::foreach_size([&]<typename M>(M) {
        EXPECT_MATRIX_EQ(IOTA(M, 0, 1).clamp(0, M::elements - 1), IOTA(M, 0, 1));
        EXPECT_MATRIX_EQ(IOTA(M, 0, 1).clamp(M(0), IOTA(M, 0, 1)), IOTA(M, 0, 1));
        EXPECT_MATRIX_EQ(IOTA(M, 0, 2).clamp(IOTA(M, 0, 1), IOTA(M, 0, 3)), IOTA(M, 0, 2));
        EXPECT_MATRIX_EQ(IOTA(M, 0, 3).clamp(IOTA(M, 0, 1), IOTA(M, 0, 2)), IOTA(M, 0, 2));
        EXPECT_MATRIX_EQ(IOTA(M, 0, 1).clamp(M(0), M(0)), M(0));
        if (std::numeric_limits<typename M::value_type>::max() >= fibn(M::elements)) {
            EXPECT_MATRIX_EQ(FIB(M, 1).clamp(M(0), IOTA(M, 0, 1)), IOTA(M, 0, 1));
        }
        if (std::numeric_limits<typename M::value_type>::max() >= sqr(M::elements - 1)) {
            EXPECT_MATRIX_EQ(SQR(M, 0, 1).clamp(M(0), IOTA(M, 0, 1)), IOTA(M, 0, 1));
        }
    });
}
// endregion
}  // namespace quicktex::tests