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

#include "BC4Encoder.h"

#include <algorithm>

namespace rgbcx {
void BC4Encoder::EncodeBlock(Byte4x4 pixels, BC4Block *const dest) const noexcept(ndebug) {
    auto bytes = pixels.Flatten();
    auto minmax = std::minmax_element(bytes.begin(), bytes.end());

    uint8_t min_v = *minmax.first;
    uint8_t max_v = *minmax.second;

    dest->high_alpha = min_v;
    dest->low_alpha = max_v;

    if (max_v == min_v) {
        dest->SetSelectorBits(0);
        return;
    }

    const uint32_t delta = max_v - min_v;

    // min_v is now 0. Compute thresholds between values by scaling max_v. It's x14 because we're adding two x7 scale factors.
    const int t0 = delta * 13;
    const int t1 = delta * 11;
    const int t2 = delta * 9;
    const int t3 = delta * 7;
    const int t4 = delta * 5;
    const int t5 = delta * 3;
    const int t6 = delta * 1;

    // BC4 floors in its divisions, which we compensate for with the 4 bias.
    // This function is optimal for all possible inputs (i.e. it outputs the same results as checking all 8 values and choosing the closest one).
    const int bias = 4 - min_v * 14;

    static const uint32_t s_tran0[8] = {1U, 7U, 6U, 5U, 4U, 3U, 2U, 0U};
    static const uint32_t s_tran1[8] = {1U << 3U, 7U << 3U, 6U << 3U, 5U << 3U, 4U << 3U, 3U << 3U, 2U << 3U, 0U << 3U};
    static const uint32_t s_tran2[8] = {1U << 6U, 7U << 6U, 6U << 6U, 5U << 6U, 4U << 6U, 3U << 6U, 2U << 6U, 0U << 6U};
    static const uint32_t s_tran3[8] = {1U << 9U, 7U << 9U, 6U << 9U, 5U << 9U, 4U << 9U, 3U << 9U, 2U << 9U, 0U << 9U};

    uint64_t a0, a1, a2, a3;
    {
        const int v0 = bytes[0] * 14 + bias;
        const int v1 = bytes[1] * 14 + bias;
        const int v2 = bytes[2] * 14 + bias;
        const int v3 = bytes[3] * 14 + bias;
        a0 = s_tran0[(v0 >= t0) + (v0 >= t1) + (v0 >= t2) + (v0 >= t3) + (v0 >= t4) + (v0 >= t5) + (v0 >= t6)];
        a1 = s_tran1[(v1 >= t0) + (v1 >= t1) + (v1 >= t2) + (v1 >= t3) + (v1 >= t4) + (v1 >= t5) + (v1 >= t6)];
        a2 = s_tran2[(v2 >= t0) + (v2 >= t1) + (v2 >= t2) + (v2 >= t3) + (v2 >= t4) + (v2 >= t5) + (v2 >= t6)];
        a3 = s_tran3[(v3 >= t0) + (v3 >= t1) + (v3 >= t2) + (v3 >= t3) + (v3 >= t4) + (v3 >= t5) + (v3 >= t6)];
    }

    {
        const int v0 = bytes[4] * 14 + bias;
        const int v1 = bytes[5] * 14 + bias;
        const int v2 = bytes[6] * 14 + bias;
        const int v3 = bytes[7] * 14 + bias;
        a0 |= (uint64_t)(s_tran0[(v0 >= t0) + (v0 >= t1) + (v0 >= t2) + (v0 >= t3) + (v0 >= t4) + (v0 >= t5) + (v0 >= t6)] << 12U);
        a1 |= (uint64_t)(s_tran1[(v1 >= t0) + (v1 >= t1) + (v1 >= t2) + (v1 >= t3) + (v1 >= t4) + (v1 >= t5) + (v1 >= t6)] << 12U);
        a2 |= (uint64_t)(s_tran2[(v2 >= t0) + (v2 >= t1) + (v2 >= t2) + (v2 >= t3) + (v2 >= t4) + (v2 >= t5) + (v2 >= t6)] << 12U);
        a3 |= (uint64_t)(s_tran3[(v3 >= t0) + (v3 >= t1) + (v3 >= t2) + (v3 >= t3) + (v3 >= t4) + (v3 >= t5) + (v3 >= t6)] << 12U);
    }

    {
        const int v0 = bytes[8] * 14 + bias;
        const int v1 = bytes[9] * 14 + bias;
        const int v2 = bytes[10] * 14 + bias;
        const int v3 = bytes[11] * 14 + bias;
        a0 |= (((uint64_t)s_tran0[(v0 >= t0) + (v0 >= t1) + (v0 >= t2) + (v0 >= t3) + (v0 >= t4) + (v0 >= t5) + (v0 >= t6)]) << 24U);
        a1 |= (((uint64_t)s_tran1[(v1 >= t0) + (v1 >= t1) + (v1 >= t2) + (v1 >= t3) + (v1 >= t4) + (v1 >= t5) + (v1 >= t6)]) << 24U);
        a2 |= (((uint64_t)s_tran2[(v2 >= t0) + (v2 >= t1) + (v2 >= t2) + (v2 >= t3) + (v2 >= t4) + (v2 >= t5) + (v2 >= t6)]) << 24U);
        a3 |= (((uint64_t)s_tran3[(v3 >= t0) + (v3 >= t1) + (v3 >= t2) + (v3 >= t3) + (v3 >= t4) + (v3 >= t5) + (v3 >= t6)]) << 24U);
    }

    {
        const int v0 = bytes[12] * 14 + bias;
        const int v1 = bytes[13] * 14 + bias;
        const int v2 = bytes[14] * 14 + bias;
        const int v3 = bytes[15] * 14 + bias;
        a0 |= (((uint64_t)s_tran0[(v0 >= t0) + (v0 >= t1) + (v0 >= t2) + (v0 >= t3) + (v0 >= t4) + (v0 >= t5) + (v0 >= t6)]) << 36U);
        a1 |= (((uint64_t)s_tran1[(v1 >= t0) + (v1 >= t1) + (v1 >= t2) + (v1 >= t3) + (v1 >= t4) + (v1 >= t5) + (v1 >= t6)]) << 36U);
        a2 |= (((uint64_t)s_tran2[(v2 >= t0) + (v2 >= t1) + (v2 >= t2) + (v2 >= t3) + (v2 >= t4) + (v2 >= t5) + (v2 >= t6)]) << 36U);
        a3 |= (((uint64_t)s_tran3[(v3 >= t0) + (v3 >= t1) + (v3 >= t2) + (v3 >= t3) + (v3 >= t4) + (v3 >= t5) + (v3 >= t6)]) << 36U);
    }

    dest->SetSelectorBits(a0 | a1 | a2 | a3);
}
}  // namespace rgbcx