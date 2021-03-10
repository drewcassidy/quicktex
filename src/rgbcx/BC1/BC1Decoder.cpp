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

#include "BC1Decoder.h"

#include <array>
#include <cassert>
#include <cstdint>

#include "../BlockView.h"
#include "../Color.h"
#include "../ndebug.h"
#include "BC1Block.h"

namespace rgbcx {
void BC1Decoder::DecodeBlock(Color4x4 dest, BC1Block *const block) const noexcept(ndebug) {
    const auto l = block->GetLowColor();
    const auto h = block->GetHighColor();
    const auto selectors = block->UnpackSelectors();
    const auto colors = _interpolator->InterpolateBC1(l, h);

    for (unsigned y = 0; y < 4; y++) {
        for (unsigned x = 0; x < 4; x++) {
            const auto selector = selectors[y][x];
            const auto color = colors[selector];
            assert(selector < 4);
            assert((color.a == 0 && selector == 3 && l <= h) || color.a == UINT8_MAX);
            if (write_alpha) {
                dest.Get(x, y).SetRGBA(color);
            } else {
                dest.Get(x, y).SetRGB(color);
            }
        }
    }
}
}  // namespace rgbcx
