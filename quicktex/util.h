/*  Quicktex Texture Compression Library
    Copyright (C) 2021-2022 Andrew Cassidy <drewcassidy@me.com>
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
#include <functional>
#include <limits>
#include <string>
#include <type_traits>
#include <vector>
#include <xsimd/xsimd.hpp>

#define UINT5_MAX 0x1FU  // 31
#define UINT6_MAX 0x3FU  // 63

#define assert5bit(x) assert(x <= UINT5_MAX)
#define assert6bit(x) assert(x <= UINT6_MAX)

namespace quicktex {

template <typename S, size_t N> S scale_from_8(S v) {
    static_assert(N < 8);
    assert(v < (1 << 8));

    unsigned max = (1 << N) - 1;
    unsigned v2 = (v * max) + 128;
    auto result = static_cast<S>((v2 + (v2 >> 8)) >> 8);

    assert(result < (1 << N));

    return result;
}

template <typename S, size_t N> S scale_to_8(S v) {
    static_assert(N < 8);
    assert(v < (1 << N));

    constexpr unsigned lshift = 8 - N;
    constexpr unsigned rshift = N - lshift;
    S result = static_cast<S>((v << lshift) | (v >> rshift));

    assert(v < (1 << 8));

    return result;
}

template <typename S> constexpr auto iabs(S i) {
    static_assert(!std::is_unsigned<S>::value);
    using O = typename std::make_unsigned<S>::type;
    return (i < 0) ? static_cast<O>(-i) : static_cast<O>(i);
}

/**
 * Unpacks an unsigned integer into an array of smaller integers.
 * @tparam I Input data type. Must be an unsigned integral type large enough to hold C * N bits.
 * @tparam O Output data type. must be an unsigned integral type large enough to hold C bits..
 * @tparam S Number of bits in each value.
 * @tparam C Number of values to unpack.
 * @param packed Packed integer input of type I.
 * @return Unpacked std::array of type O and size C.
 */
template <typename I, typename O, size_t S, size_t C> constexpr std::array<O, C> Unpack(I packed) {
    // type checking
    static_assert(std::is_unsigned<I>::value, "Packed input type must be unsigned");
    static_assert(std::is_unsigned<O>::value, "Unpacked output type must be unsigned");
    static_assert(std::numeric_limits<I>::digits >= (C * S),
                  "Packed input type must be big enough to represent the number of bits multiplied by count");
    static_assert(std::numeric_limits<O>::digits >= S,
                  "Unpacked output type must be big enough to represent the number of bits");

    constexpr O mask = (1U << S) - 1U;  // maximum value representable by N bits
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
 * @tparam O Output data type. must be an unsigned integral type large enough to hold C * N bits.
 * @tparam S Number of bits in each value.
 * @tparam C Number of values to unpack.
 * @param vals Unpacked std::array of type I and size C.
 * @return Packed integer input of type O.
 */
template <typename I, typename O, size_t S, size_t C> constexpr O Pack(const std::array<I, C> &vals) {
    // type checking
    static_assert(std::is_unsigned<I>::value, "Unpacked input type must be unsigned");
    static_assert(std::is_unsigned<O>::value, "Packed output type must be unsigned");
    static_assert(std::numeric_limits<I>::digits >= S,
                  "Unpacked input type must be big enough to represent the number of bits");
    static_assert(std::numeric_limits<O>::digits >= (C * S),
                  "Packed output type must be big enough to represent the number of bits multiplied by count");

    O packed = 0;  // output value of type O

    for (unsigned i = 0; i < C; i++) {
        assert(vals[i] <= (1U << S) - 1U);
        packed |= static_cast<O>(vals[i]) << (i * S);
    }

    assert(packed <= (static_cast<O>(1U) << (C * S)) - 1U);
    return packed;
}

template <size_t Size, int Op(int)> constexpr std::array<uint8_t, Size> ExpandArray() {
    std::array<uint8_t, Size> res;
    for (int i = 0; i < Size; i++) { res[i] = Op(i); }
    return res;
}

template <typename Seq, typename Fn> constexpr auto MapArray(const Seq &input, Fn op) {
    using I = typename Seq::value_type;
    using O = decltype(op(std::declval<I>()));
    constexpr size_t N = std::tuple_size<Seq>::value;

    std::array<O, N> output;
    for (unsigned i = 0; i < N; i++) { output[i] = op(input[i]); }
    return output;
}

template <typename S> constexpr S scale8To5(S v) {
    auto v2 = v * 31 + 128;
    return static_cast<S>((v2 + (v2 >> 8)) >> 8);
}
template <typename S> constexpr S scale8To6(S v) {
    auto v2 = v * 63 + 128;
    return static_cast<S>((v2 + (v2 >> 8)) >> 8);
}

template <typename S> constexpr S scale5To8(S v) {
    assert5bit(v);
    return static_cast<S>((v << 3) | (v >> 2));
}
template <typename S> constexpr S scale6To8(S v) {
    assert6bit(v);
    return static_cast<S>((v << 2) | (v >> 4));
}

template <typename S> constexpr S clamp(S value, S low, S high) {
    assert(low <= high);
    if (value < low) return low;
    if (value > high) return high;
    return value;
}
constexpr int32_t clampi(int32_t value, int32_t low, int32_t high) {
    if (value < low)
        value = low;
    else if (value > high)
        value = high;
    return value;
}

template <typename T> std::enable_if<std::is_unsigned_v<T>, T> abs(const T &sval) { return sval; }
template <typename T> std::enable_if<std::is_signed_v<T> && std::is_arithmetic_v<T>, T> abs(const T &a) {
    return (a < 0) ? -a : a;
}
using xsimd::abs;  // provides overload for abs<xsimd::batch>

template <typename F> constexpr F lerp(F a, F b, F s) { return a + (b - a) * s; }

template <typename... Args> std::string Format(const char *str, const Args &...args) {
    auto output = std::string(str);

    std::vector<std::string> values = {{args...}};

    for (unsigned i = 0; i < values.size(); i++) {
        auto key = "{" + std::to_string(i) + "}";
        auto value = values[i];
        while (true) {
            size_t where = output.find(key);
            if (where == output.npos) break;
            output.replace(where, key.length(), value);
        }
    }

    return output;
}

template <class> struct next_size;
template <class T> using next_size_t = typename next_size<T>::type;
template <class T> struct Tag { using type = T; };

template <> struct next_size<int8_t> : Tag<int16_t> {};
template <> struct next_size<int16_t> : Tag<int32_t> {};
template <> struct next_size<int32_t> : Tag<int64_t> {};

template <> struct next_size<uint8_t> : Tag<uint16_t> {};
template <> struct next_size<uint16_t> : Tag<uint32_t> {};
template <> struct next_size<uint32_t> : Tag<uint64_t> {};
}  // namespace quicktex