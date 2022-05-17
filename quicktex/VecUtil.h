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
#include <hwy/highway.h>
namespace hn = hwy::HWY_NAMESPACE;

#if HWY_TARGET == HWY_NEON
#include <arm_neon.h>
#elif HWY_ARCH_X86_64
#include <immintrin.h>
#endif

namespace quicktex {

using Tag_s16 = hn::ScalableTag<int16_t>;
using Vec_s16 = hn::Vec<Tag_s16>;
using Tag_s32 = hn::ScalableTag<int32_t>;
using Vec_s32 = hn::Vec<Tag_s32>;

const Tag_s16 TagS16;
const Tag_s32 TagS32;

/// Helper function for doing sum-of-lanes without a tag lvalue. We're not targeting SVE (yet) so this should work fine.
/// \tparam V Vector type to sum (8- and 16-bit integers are NOT supported)
/// \param v Vector to sum
/// \return The sum of all lanes in each lane.
template <typename V> inline V SumOfLanes(V v) {
    hn::DFromV<V> tag;
    return hn::SumOfLanes(tag, v);
}

inline int32_t WideningSumS16(const Vec_s16 v) {
#if HWY_TARGET == HWY_SCALAR
    // In Scalar mode this is a no-op, since there's only one lane
    return (int32_t)v.raw;
#elif HWY_TARGET == HWY_EMU128
    // In emulated 128-bit mode, do the addition serially
    int acc = 0;
    for (unsigned i = 0; i < hn::MaxLanes(TagS16); i++) { acc += v.raw[i]; }
    return acc;
#elif HWY_TARGET == HWY_NEON
    static_assert(hn::MaxLanes(TagS16) == 8);
    static_assert(hn::MaxLanes(TagS32) == 4);

    // Pairwise widening sum, then sum all N/2 widened lanes
    auto paired = Vec_s32(vpaddlq_s16(v.raw));
    auto sums = SumOfLanes(paired);
    return hn::GetLane(sums);
#elif HWY_ARCH_X86_64
#if HWY_TARGET == HWY_AVX2 || HWY_TARGET == HWY_AVX3
    static_assert(hn::MaxLanes(TagS16) == 16);
    static_assert(hn::MaxLanes(TagS32) == 8);

    // Pairwise widening sum with multiply by 1, then sum all N/2 widened lanes
    auto paired = Vec_s32{_mm256_madd_epi16(v.raw, _mm256_set1_epi16(1))};
    auto sums = SumOfLanes(paired);
    return hn::GetLane(sums);
#else
    static_assert(hn::MaxLanes(TagS16) == 8);
    static_assert(hn::MaxLanes(TagS32) == 4);

    // Pairwise widening sum with multiply by 1, then sum all N/2 widened lanes
    auto paired = Vec_s32{_mm_madd_epi16(v.raw, _mm_set1_epi16(1))};
    auto sums = SumOfLanes(paired);
    return hn::GetLane(sums);
#endif
#endif
}

}  // namespace quicktex