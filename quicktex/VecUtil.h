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

template <typename T> using requires_arch = xsimd::kernel::requires_arch<T>;

namespace quicktex::simd {

namespace kernel {

#if XSIMD_WITH_NEON64
template <class A> inline int16_t whadd(xsimd::batch<int8_t, A> const& arg, requires_arch<xsimd::neon64>) {
    return vaddlvq_s8(arg);
}

template <class A> inline int32_t whadd(xsimd::batch<int16_t, A> const& arg, requires_arch<xsimd::neon64>) {
    return vaddlvq_s16(arg);
}

template <class A> inline int64_t whadd(xsimd::batch<int32_t, A> const& arg, requires_arch<xsimd::neon64>) {
    return vaddlvq_s32(arg);
}

template <class A> inline uint16_t whadd(xsimd::batch<uint8_t, A> const& arg, requires_arch<xsimd::neon64>) {
    return vaddlvq_u8(arg);
}

template <class A> inline uint32_t whadd(xsimd::batch<uint16_t, A> const& arg, requires_arch<xsimd::neon64>) {
    return vaddlvq_u16(arg);
}

template <class A> inline uint64_t whadd(xsimd::batch<uint32_t, A> const& arg, requires_arch<xsimd::neon64>) {
    return vaddlvq_u32(arg);
}
#endif

#if XSIMD_WITH_SSE2
template <class A> inline int32_t whadd(xsimd::batch<int16_t, A> const& arg, requires_arch<xsimd::sse2>) {
    // Pairwise widening sum with multiply by 1, then sum all N/2 widened lanes
    xsimd::batch<int32_t, A> paired = _mm_madd_epi16(arg, _mm_set1_epi16(1));
    return xsimd::hadd(paired);
}
#endif

#if XSIMD_WITH_AVX2
template <class A> inline int32_t whadd(xsimd::batch<int16_t, A> const& arg, requires_arch<xsimd::avx2>) {
    // Pairwise widening sum with multiply by 1, then sum all N/2 widened lanes
    xsimd::batch<int32_t, A> paired = _mm256_madd_epi16(arg, _mm256_set1_epi16(1));
    return xsimd::hadd(paired);
}
#endif

template <class A, class T> inline next_size_t<T> whadd(xsimd::batch<T, A> const& arg, requires_arch<xsimd::generic>) {
    // Generic implementation that should work everywhere
    using b_type = xsimd::batch<T, A>;
    using r_type = next_size_t<T>;
    const auto len = b_type::size;

    alignas(A::alignment()) T buffer[len];
    r_type sum = 0;

    arg.store_aligned(buffer);
    for (T val : buffer) { sum += static_cast<r_type>(val); }

    return sum;
}
}  // namespace kernel

template <class A, class T> inline next_size_t<T> whadd(xsimd::batch<T, A> const& arg) {
    return kernel::whadd(arg, A{});
}

}  // namespace quicktex::simd