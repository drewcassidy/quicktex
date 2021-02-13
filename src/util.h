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

#pragma once
#include <cassert>
#include <cstdint>
#include <limits>
#include <type_traits>

#include "ndebug.h"

#define UINT5_MAX 0x1FU  // 31
#define UINT6_MAX 0x3FU  // 63

template <typename S> constexpr void Assert5Bit(S x) {
    static_assert(std::is_unsigned<S>::value);
    assert(x <= UINT5_MAX);
}
template <typename S> constexpr void Assert6Bit(S x) {
    static_assert(std::is_unsigned<S>::value);
    assert(x <= UINT6_MAX);
}

template <typename S> constexpr auto iabs(S i) {
    using O = typename std::make_unsigned<S>::type;
    return (i < 0) ? static_cast<O>(-i) : static_cast<O>(i);
}

/**
 * Unpacks an unsigned integer into an array of smaller integers.
 * @tparam I Input data type. Must be an unsigned integral type large enough to hold C * S bits.
 * @tparam O Output data type. must be an unsigned integral type large enough to hold C bits..
 * @tparam S Number of bits in each value.
 * @tparam C Number of values to unpack.
 * @param packed Packed integer input of type I.
 * @return Unpacked std::array of type O and size C.
 */
template <typename I, typename O, size_t S, size_t C> constexpr auto Unpack(I packed) noexcept(ndebug) {
    // type checking
    static_assert(std::is_unsigned<I>::value, "Packed input type must be unsigned");
    static_assert(std::is_unsigned<O>::value, "Unpacked output type must be unsigned");
    static_assert(std::numeric_limits<I>::digits >= (C * S), "Packed input type must be big enough to represent the number of bits multiplied by count");
    static_assert(std::numeric_limits<O>::digits >= S, "Unpacked output type must be big enough to represent the number of bits");

    constexpr O mask = (1U << S) - 1U;  // maximum value representable by S bits
    std::array<O, C> vals;              // output values array of size C

    for (unsigned i = 0; i < C; i++) {
        vals[i] = static_cast<O>(packed >> (i * S)) & mask;
        assert(vals[i] <= mask);
    }

    return vals;
}

/**
 * Packs an array of unsigned integers into a single integer.
 * @tparam I Input data type. Must be an unsigned integral type large enough to hold C bits.
 * @tparam O Output data type. must be an unsigned integral type large enough to hold C * S bits.
 * @tparam S Number of bits in each value.
 * @tparam C Number of values to unpack.
 * @param vals Unpacked std::array of type I and size C.
 * @return Packed integer input of type O.
 */
template <typename I, typename O, size_t S, size_t C> constexpr auto Pack(const std::array<I, C> &vals) noexcept(ndebug) {
    // type checking
    static_assert(std::is_unsigned<I>::value, "Unpacked input type must be unsigned");
    static_assert(std::is_unsigned<O>::value, "Packed output type must be unsigned");
    static_assert(std::numeric_limits<I>::digits >= S, "Unpacked input type must be big enough to represent the number of bits");
    static_assert(std::numeric_limits<O>::digits >= (C * S), "Packed output type must be big enough to represent the number of bits multiplied by count");

    constexpr I max_input = (1U << S) - 1U;                         // maximum value representable by S bits
    constexpr O max_output = (static_cast<O>(1U) << (C * S)) - 1U;  // maximum value representable by S * C bits
    O packed = 0;                                                   // output value of type O

    for (unsigned i = 0; i < C; i++) {
        assert(vals[i] <= max_input);
        packed |= static_cast<O>(vals[i]) << (i * S);
    }

    assert(packed <= max_output);
    return packed;
}

template <size_t Size, int Op(int)> constexpr std::array<uint8_t, Size> ExpandArray() {
    std::array<uint8_t, Size> res;
    for (int i = 0; i < Size; i++) { res[i] = Op(i); }
    return res;
}

template <typename S> constexpr S scale8To5(S v) {
    v = v * 31 + 128;
    return static_cast<S>((v + (v >> 8)) >> 8);
}
template <typename S> constexpr S scale8To6(S v) {
    v = v * 63 + 128;
    return static_cast<S>((v + (v >> 8)) >> 8);
}

template <typename S> constexpr S scale5To8(S v) {
    Assert5Bit(v);
    assert(v <= UINT5_MAX);
    return static_cast<S>((v << 3) | (v >> 2));
}
template <typename S> constexpr S scale6To8(S v) {
    Assert6Bit(v);
    return static_cast<S>((v << 2) | (v >> 4));
}

template <typename S> constexpr S maximum(S a, S b) { return (a > b) ? a : b; }
template <typename S> constexpr S maximum(S a, S b, S c) { return maximum(maximum(a, b), c); }
template <typename S> constexpr S maximum(S a, S b, S c, S d) { return maximum(maximum(maximum(a, b), c), d); }

template <typename S> constexpr S minimum(S a, S b) { return (a < b) ? a : b; }
template <typename S> constexpr S minimum(S a, S b, S c) { return minimum(minimum(a, b), c); }
template <typename S> constexpr S minimum(S a, S b, S c, S d) { return minimum(minimum(minimum(a, b), c), d); }

template <typename T> constexpr T square(T a) { return a * a; }

constexpr float clampf(float value, float low, float high) {
    if (value < low)
        value = low;
    else if (value > high)
        value = high;
    return value;
}
constexpr uint8_t clamp255(int32_t i) { return static_cast<uint8_t>((static_cast<unsigned int>(i) & 0xFFFFFF00U) ? (~(i >> 31)) : i); }

template <typename S> constexpr S clamp(S value, S low, S high) { return (value < low) ? low : ((value > high) ? high : value); }
constexpr int32_t clampi(int32_t value, int32_t low, int32_t high) {
    if (value < low)
        value = low;
    else if (value > high)
        value = high;
    return value;
}

constexpr int squarei(int a) { return a * a; }
constexpr int absi(int a) { return (a < 0) ? -a : a; }

template <typename F> constexpr F lerp(F a, F b, F s) { return a + (b - a) * s; }
