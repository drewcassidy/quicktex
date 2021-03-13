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
#include <array>
#include <cstdint>
#include <utility>

#include "../../BlockView.h"
#include "../../formats/blocks/BC4Block.h"
#include "../../ndebug.h"

namespace quicktex {
void BC4Encoder::EncodeBlock(Byte4x4 pixels, BC4Block *const dest) const noexcept(ndebug) {
    auto flattened = pixels.Flatten();
    auto minmax = std::minmax_element(flattened.begin(), flattened.end());

    uint8_t min = *minmax.first;
    uint8_t max = *minmax.second;

    dest->high_alpha = min;
    dest->low_alpha = max;

    if (max == min) {
        dest->SetSelectorBits(0);
        return;
    }

    std::array<uint8_t, 16> selectors = {};
    const static std::array<uint8_t, 8> Levels = {1U, 7U, 6U, 5U, 4U, 3U, 2U, 0U};  // selector value options in linear order

    // BC4 floors in its divisions, which we compensate for with the 4 bias.
    // This function is optimal for all possible inputs (i.e. it outputs the same results as checking all 8 values and choosing the closest one).
    const int bias = 4 - min * 14;
    const int delta = max - min;

    // Min is now 0. Compute thresholds between values by scaling max. It's x14 because we're adding two x7 scale factors.
    // bias is applied here
    std::array<int, 7> thresholds = {};
    for (unsigned i = 0; i < 7; i++) thresholds[i] = delta * (1 + (2 * (int)i)) - bias;

    // iterate over all values and calculate selectors
    for (unsigned i = 0; i < 16; i++) {
        int value = flattened[i] * 14;  // multiply by demonimator

        // level = number of thresholds this value is greater than
        unsigned level = 0;
        for (unsigned c = 0; c < 7; c++) level += value >= thresholds[c];

        selectors[i] = Levels[level];
    }

    dest->PackSelectors(selectors);
}
}  // namespace quicktex