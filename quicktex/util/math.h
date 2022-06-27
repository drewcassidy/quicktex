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
#include <numeric>
#include <string>
#include <type_traits>
#include <vector>

#include "util/ranges.h"
#include "xsimd/xsimd.hpp"

namespace quicktex {

using std::abs;    // abs overload for builtin types
using xsimd::abs;  // abs overload for xsimd buffers

template <typename S>
    requires requires(S &s) { s.abs(); }
constexpr S abs(S value) {
    return value.abs();
}

template <typename S, typename R>
    requires requires(S s, R r) { s.clamp(r, r); }
constexpr S clamp(S value, R low, R high) {
    return value.clamp(low, high);
}

template <typename S>
    requires std::is_scalar_v<S>
constexpr S clamp(S value, S low, S high) {
    assert(low <= high);
    if (value < low) return low;
    if (value > high) return high;
    return value;
}

template <typename S, typename A>
constexpr xsimd::batch<S, A> clamp(xsimd::batch<S, A> value, const xsimd::batch<S, A> &low,
                                   const xsimd::batch<S, A> &high) {
    return xsimd::clip(value, low, high);
}

template <typename S, typename A>
constexpr xsimd::batch<S, A> clamp(xsimd::batch<S, A> value, const S &low, const S &high) {
    return clamp(value, xsimd::broadcast(low), xsimd::broadcast(high));
}

template <typename S>
    requires requires(S &s) { s.sum(); }
constexpr auto sum(S value) {
    return value.sum();
}

template <typename S>
    requires std::is_scalar_v<S>
constexpr auto sum(S value) {
    return value;
    // horizontally adding a scalar is a noop
}

template <typename S, typename A> constexpr auto sum(xsimd::batch<S, A> value) { return xsimd::hadd(value); }
}  // namespace quicktex