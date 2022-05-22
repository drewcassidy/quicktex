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
#include <type_traits>
#include <xsimd/xsimd.hpp>

#include "util.h"

namespace quicktex {
template <class A, class T> inline next_size_t<T> widening_hadd(xsimd::batch<T, A> const& arg) {
    using b_type = xsimd::batch<T, A>;
    using r_type = next_size_t<T>;
    const auto len = b_type::size;

    std::array<T, len> buff;
    r_type sum = 0;

    arg.store(&buff[0]);
    for (unsigned i = 0; i < len; i++) { sum += static_cast<r_type>(buff[i]); }

    return sum;
}

#if XSIMD_WITH_NEON64
template <> inline int32_t widening_hadd(xsimd::batch<int16_t, xsimd::neon64> const& arg) {
    // Pairwise widening sum, then sum all N/2 widened lanes
    xsimd::batch<int32_t, xsimd::neon64> paired = vpaddlq_s16(arg);
    return xsimd::hadd(paired);
}
#endif

#if XSIMD_WITH_SSE2
template <> inline int32_t widening_hadd(xsimd::batch<int16_t, xsimd::sse2> const& arg) {
    // Pairwise widening sum with multiply by 1, then sum all N/2 widened lanes
    xsimd::batch<int32_t, xsimd::sse2> paired = _mm_madd_epi16(arg, _mm_set1_epi16(1));
    return xsimd::hadd(paired);
}
#endif

#if XSIMD_WITH_AVX2
template <> inline int32_t widening_hadd(xsimd::batch<int16_t, xsimd::avx2> const& arg) {
    // Pairwise widening sum with multiply by 1, then sum all N/2 widened lanes
    xsimd::batch<int32_t, xsimd::avx2> paired = _mm256_madd_epi16(arg, _mm256_set1_epi16(1));
    return xsimd::hadd(paired);
}
#endif

}  // namespace quicktex