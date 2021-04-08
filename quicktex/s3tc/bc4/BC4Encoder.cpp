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

#include "BC4Encoder.h"

#include <algorithm>
#include <array>
#include <cstdint>

#include "../../Color.h"
#include "../../ColorBlock.h"
#include "BC4Block.h"

namespace quicktex::s3tc {
BC4Block BC4Encoder::EncodeBlock(const ColorBlock<4, 4> &pixels) const {
    uint8_t min = UINT8_MAX;
    uint8_t max = 0;

    for (int i = 0; i < 16; i++) {
        auto value = pixels.Get(i)[_channel];
        min = std::min(min, value);
        max = std::max(max, value);
    }

    if (max == min) {
        return BC4Block(min);  // solid block
    }

    auto selectors = BC4Block::SelectorArray();
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
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            int value = (int)pixels.Get(x, y)[_channel] * 14;  // multiply by demonimator

            // level = number of thresholds this value is greater than
            unsigned level = 0;
            for (unsigned c = 0; c < 7; c++) level += value >= thresholds[c];

            selectors[(unsigned)y][(unsigned)x] = Levels[level];
        }
    }

    return BC4Block(max, min, selectors);
}

}  // namespace quicktex::s3tc