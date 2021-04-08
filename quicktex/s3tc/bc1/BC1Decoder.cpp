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

#include "BC1Decoder.h"

#include <array>
#include <cassert>
#include <cstdint>

#include "../../Color.h"
#include "../../ColorBlock.h"
#include "BC1Block.h"

namespace quicktex::s3tc {

ColorBlock<4, 4> BC1Decoder::DecodeBlock(const BC1Block &block) const { return DecodeBlock(block, true); }

ColorBlock<4, 4> BC1Decoder::DecodeBlock(const BC1Block &block, bool use_3color) const {
    auto output = ColorBlock<4, 4>();
    const auto l = block.GetColor0Raw();
    const auto h = block.GetColor1Raw();
    const auto selectors = block.GetSelectors();
    const auto colors = _interpolator->Interpolate565BC1(l, h, use_3color);

    for (unsigned y = 0; y < 4; y++) {
        for (unsigned x = 0; x < 4; x++) {
            const auto selector = selectors[y][x];
            auto color = colors[selector];
            assert(selector < 4);
            assert((color.a == 0 && selector == 3 && l <= h) || color.a == UINT8_MAX);
            if (!write_alpha) { color.a = output.Get(x, y).a; }
            output.Set(x, y, color);
        }
    }

    return output;
}
}  // namespace quicktex::s3tc
