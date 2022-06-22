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

namespace detail {
using std::abs;    // abs overload for builtin types
using xsimd::abs;  // abs overload for xsimd buffers
}  // namespace detail

template <typename S>
    requires requires(S &s) { s.abs(); }
constexpr S abs(S value) {
    return value.abs();
}

template <typename S>
    requires requires(S &s) { detail::abs(s); }
constexpr S abs(S value) {
    return detail::abs(value);
}

template <typename S>
    requires requires(S &s) { s.clamp(s, s); }
constexpr S clamp(S value, S low, S high) {
    assert(low <= high);
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
constexpr S clamp(xsimd::batch<S, A> value, const xsimd::batch<S, A> &low, const xsimd::batch<S, A> &high) {
    value = xsimd::select(xsimd::lt(low), low, value);
    value = xsimd::select(xsimd::gt(high), high, value);
    return value;
}

}  // namespace quicktex