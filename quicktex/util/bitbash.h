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
#include <cassert>
#include <concepts>
#include <limits>
#include <numeric>
#include <type_traits>

#include "util/math.h"
#include "util/ranges.h"

#define UINT5_MAX 0x1FU  // 31
#define UINT6_MAX 0x3FU  // 63

#define assert5bit(x) assert(x <= UINT5_MAX)
#define assert6bit(x) assert(x <= UINT6_MAX)

namespace quicktex {

template <size_t N, typename S> S scale_from_8(S v) {
    static_assert(N < 8);
    assert(v < (1 << 8));

    unsigned max = (1 << N) - 1;
    unsigned v2 = (v * max) + 128;
    auto result = static_cast<S>((v2 + (v2 >> 8)) >> 8);

    assert(result < (1 << N));

    return result;
}

template <size_t N, typename S> S scale_to_8(S v) {
    static_assert(N < 8);
    assert(v < (1 << N));

    constexpr unsigned Lshift = 8 - N;
    constexpr unsigned Rshift = N - Lshift;
    S result = static_cast<S>((v << Lshift) | (v >> Rshift));

    assert(v < (1 << 8));

    return result;
}

/**
 * Unpacks an unsigned integer into a range of smaller integers.
 * @param packed value to unpack
 * @param begin destination start iterator
 * @param end destination end iterator
 * @param widths widths iterator. values are in bits
 * @param little_endian if the input has the first element in the least significant place
 * @return the total number of bits unpacked
 */
template <typename P, typename OI, typename WI>
    requires std::unsigned_integral<P> && std::output_iterator<OI, P> && std::forward_iterator<WI>
size_t unpack_into(P packed, OI begin, OI end, WI widths, bool little_endian = true) {
    using U = std::remove_cvref_t<decltype(*begin)>;
    if (little_endian) {
        // first element is in the least significant place of packed

        unsigned offset = 0;
        while (begin < end) {
            auto w = *(widths++);
            assert(w <= std::numeric_limits<U>::digits);

            auto mask = ((1 << w) - 1);              // least significant w bits all 1
            *(begin++) = (packed >> offset) & mask;  // write to output

            offset += w;  // increment offset
        }

        assert(offset <= std::numeric_limits<P>::digits);  // detect an overflow condition
        return offset;
    } else {
        // first element is in the most significant place of packed

        // with non-constant width, we either need to iterate backwards or
        // add up all the widths beforehand to know where to begin
        unsigned total_offset = std::accumulate(widths, widths + std::distance(begin, end), 0);
        assert(total_offset <= std::numeric_limits<P>::digits);  // detect an overflow condition

        unsigned offset = total_offset;
        while (begin < end) {
            auto w = *(widths++);
            offset -= w;                                 // decrement offset
            assert(w < std::numeric_limits<U>::digits);  // detect an overflow condition

            auto mask = ((1 << w) - 1);              // least significant w bits all 1
            *(begin++) = (packed >> offset) & mask;  // write to output
        }

        return total_offset;
    }
}

/**
 * Unpacks an unsigned integer into a range of smaller integers.
 * @param packed value to unpack
 * @param dest destination range
 * @param widths widths range. values are in bits
 * @param little_endian if the input has the first element in the least significant place
 * @return the total number of bits unpacked
 */
template <typename P, typename OR, typename WR>
    requires std::unsigned_integral<P> && range<OR> && range<WR>
size_t unpack_into(P packed, OR &dest, const WR &widths, bool little_endian = true) {
    assert(distance(widths) == distance(dest));
    return unpack_into(packed, dest.begin(), dest.end(), widths.begin(), little_endian);
}

/**
 * Unpacks an unsigned integer into a range of smaller integers.
 * @param packed value to unpack
 * @param begin destination start iterator
 * @param end destination end iterator
 * @param width width of each packed element in bits
 * @param little_endian if the input has the first element in the least significant place
 * @return the total number of bits unpacked
 */
template <typename P, typename OI>
    requires std::unsigned_integral<P> && std::output_iterator<OI, P>
size_t unpack_into(P packed, OI begin, OI end, size_t width, bool little_endian = true) {
    return unpack_into(packed, begin, end, const_iterator(width), little_endian);
}

/**
 * Unpacks an unsigned integer into a range of smaller integers.
 * @param packed value to unpack
 * @param dest destination range
 * @param width width of each packed element in bits
 * @param little_endian if the input has the first element in the least significant place
 * @return the total number of bits unpacked
 */
template <typename P, typename OR>
    requires std::unsigned_integral<P> && range<OR>
size_t unpack_into(P packed, OR &dest, size_t width, bool little_endian = true) {
    return unpack_into(packed, dest.begin(), dest.end(), const_iterator(width), little_endian);
}

/**
 * Unpacks an unsigned integer into an array of smaller integers
 * @tparam U unpacked data type
 * @tparam N number of values to unpack
 * @param packed value to unpack
 * @param widths widths iterator. values are in bits
 * @param little_endian if the input has the first element in the least significant place
 * @return an array of unpacked values
 */
template <typename U, size_t N, typename P, typename WI>
    requires std::unsigned_integral<P> && std::forward_iterator<WI>
std::array<U, N> unpack(P packed, WI widths, bool little_endian = true) {
    std::array<U, N> unpacked;
    unpack_into(packed, unpacked, widths, little_endian);
    return unpacked;
}

/**
 * Unpacks an unsigned integer into an array of smaller integers
 * @tparam U unpacked data type
 * @param packed value to unpack
 * @param widths widths array. values are in bits
 * @param little_endian if the input has the first element in the least significant place
 * @return an array of unpacked values
 */
template <typename U, size_t N, typename P>
    requires std::unsigned_integral<P>
std::array<U, N> unpack(P packed, const std::array<size_t, N> &widths, bool little_endian = true) {
    return unpack<U, N>(packed, widths.begin(), little_endian);
}

/**
 * Unpacks an unsigned integer into an array of smaller integers
 * @tparam U unpacked data type
 * @tparam N number of values to unpack
 * @param packed value to unpack
 * @param widths widths range. values are in bits
 * @param little_endian if the input has the first element in the least significant place
 * @return an array of unpacked values
 */
template <typename U, size_t N, typename P, typename WR>
    requires std::unsigned_integral<P> && range<WR>
std::array<U, N> unpack(P packed, const WR &widths, bool little_endian = true) {
    assert(distance(widths) == N);
    return unpack<U, N>(packed, widths.begin(), little_endian);
}

/**
 * Unpacks an unsigned integer into an array of smaller integers
 * @tparam U unpacked data type
 * @tparam N number of values to unpack
 * @param packed value to unpack
 * @param width width of each packed element in bits
 * @param little_endian if the input has the first element in the least significant place
 * @return an array of unpacked values
 */
template <typename U, size_t N, typename P>
    requires std::unsigned_integral<P>
std::array<U, N> unpack(P packed, size_t width, bool little_endian = true) {
    std::array<U, N> unpacked;
    unpack_into(packed, unpacked, width, little_endian);
    return unpacked;
}

/**
 * Packs an iterable of integers into a single integer.
 * @tparam II input iterator type
 * @tparam WI width iterator type
 * @tparam P Output data type. must be an unsigned integral type large enough to hold all input values
 * @param start start iterator
 * @param end end iterator
 * @param widths width iterator. must be at least as large as the input data
 * @param little_endian if the output value should have the first element in the least significant place
 * of the output or not
 * @return Packed integer of type P.
 */
template <typename P, typename II, typename WI>
    requires std::unsigned_integral<P> && std::input_iterator<II> && std::input_iterator<WI>
inline constexpr P pack(II start, II end, WI widths, bool little_endian = true) {
    P packed = 0;
    unsigned offset = 0;
    while (start < end) {
        auto val = static_cast<P>(*(start++));
        auto w = *(widths++);

        val &= ((1 << w) - 1);
        assert(val < (1 << w));  // ensure value can fit in W bits

        if (little_endian) {
            packed |= static_cast<P>(val) << offset;  // first element is in the least significant place of packed
        } else {
            packed = (packed << w) | static_cast<P>(val);  // first element is in the most significant place of packed
        }

        offset += w;  // increment offset
    }

    assert(offset <= std::numeric_limits<P>::digits);  // detect an overflow condition
    return packed;
}

/**
 * Packs an iterable of integers into a single integer.
 * @tparam IR input range type
 * @tparam WR width range type
 * @tparam P Output data type. must be an unsigned integral type large enough to hold all input values
 * @param r range of values to pack
 * @param widths range of widths to pack with. must be at least as large as r
 * @param little_endian if the output value should have the first element in the least significant place
 * of the output or not
 * @return Packed integer of type P.
 */
template <typename P, typename IR, typename WR>
    requires std::unsigned_integral<P> && range<IR> && range<WR>
inline constexpr P pack(IR r, WR widths, bool little_endian = true) {
    assert(distance(widths) == distance(r));
    return pack<P>(r.begin(), r.end(), widths.start(), little_endian);
}

/**
 * Packs an iterable of integers into a single integer.
 * @tparam II input iterator type
 * @tparam P Output data type. must be an unsigned integral type large enough to hold all input values
 * @param start start iterator
 * @param end end iterator
 * @param width Number of bits in each value
 * @param little_endian if the output value should have the first element in the least significant place
 * of the output or not
 * @return Packed integer of type P.
 */
template <typename P, typename II>
    requires std::unsigned_integral<P> && std::input_iterator<II>
inline constexpr P pack(II start, II end, size_t width, bool little_endian = true) {
    return pack<P>(start, end, const_iterator(width), little_endian);
}

/**
 * Packs a range of integers into a single integer.
 * @tparam IR range type
 * @tparam P Output data type. must be an unsigned integral type large enough to hold all input values
 * @param r range of values to pack
 * @param width Number of bits in each value
 * @param little_endian if the output value should have the first element in the least significant place
 * of the output or not
 * @return Packed integer of type P.
 */
template <typename P, typename IR>
    requires std::unsigned_integral<P> && range<IR>
inline constexpr P pack(IR r, size_t width, bool little_endian = true) {
    return pack<P>(r.begin(), r.end(), const_iterator(width), little_endian);
}
}  // namespace quicktex