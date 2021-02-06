/*  Python-rgbcx Texture Compression Library
    Copyright (C) 2021 Andrew Cassidy <drewcassidy@me.com>
    Partially derived from rgbcx.h written by Richard Geldreich <richgel99@gmail.com>
    and licenced under the public domain

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#include <cstdint>

static inline uint32_t iabs(int32_t i) { return (i < 0) ? static_cast<uint32_t>(-i) : static_cast<uint32_t>(i); }
static inline uint64_t iabs(int64_t i) { return (i < 0) ? static_cast<uint64_t>(-i) : static_cast<uint64_t>(i); }

static inline uint8_t scale8To5(uint32_t v) {
    v = v * 31 + 128;
    return (uint8_t)((v + (v >> 8)) >> 8);
}
static inline uint8_t scale8To6(uint32_t v) {
    v = v * 63 + 128;
    return (uint8_t)((v + (v >> 8)) >> 8);
}

static inline int scale5To8(int v) { return (v << 3) | (v >> 2); }
static inline int scale6To8(int v) { return (v << 2) | (v >> 4); }

template <typename S> inline S maximum(S a, S b) { return (a > b) ? a : b; }
template <typename S> inline S maximum(S a, S b, S c) { return maximum(maximum(a, b), c); }
template <typename S> inline S maximum(S a, S b, S c, S d) { return maximum(maximum(maximum(a, b), c), d); }

template <typename S> inline S minimum(S a, S b) { return (a < b) ? a : b; }
template <typename S> inline S minimum(S a, S b, S c) { return minimum(minimum(a, b), c); }
template <typename S> inline S minimum(S a, S b, S c, S d) { return minimum(minimum(minimum(a, b), c), d); }

template <typename T> inline T square(T a) { return a * a; }

static inline float clampf(float value, float low, float high) {
    if (value < low)
        value = low;
    else if (value > high)
        value = high;
    return value;
}
static inline uint8_t clamp255(int32_t i) { return (uint8_t)((i & 0xFFFFFF00U) ? (~(i >> 31)) : i); }

template <typename S> inline S clamp(S value, S low, S high) { return (value < low) ? low : ((value > high) ? high : value); }
static inline int32_t          clampi(int32_t value, int32_t low, int32_t high) {
    if (value < low)
        value = low;
    else if (value > high)
        value = high;
    return value;
}

static inline int squarei(int a) { return a * a; }
static inline int absi(int a) { return (a < 0) ? -a : a; }

template <typename F> inline F lerp(F a, F b, F s) { return a + (b - a) * s; }
