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

#include "BC4Decoder.h"

#include <array>    // for array
#include <cassert>  // for assert

#include "../../Color.h"
#include "../../ColorBlock.h"
#include "BC4Block.h"

namespace quicktex::s3tc {
void BC4Decoder::DecodeInto(ColorBlock<4, 4> &dest, const BC4Block &block) const {
    auto values = block.GetValues();
    auto selectors = block.GetSelectors();

    for (unsigned y = 0; y < 4; y++) {
        for (unsigned x = 0; x < 4; x++) {
            const auto selector = selectors[y][x];
            assert(selector < 8);

            auto color = dest.Get(x, y);
            color[_channel] = values[selector];
            dest.Set(x, y, color);
        }
    }
}

ColorBlock<4, 4> BC4Decoder::DecodeBlock(const BC4Block &block) const {
    auto output = ColorBlock<4, 4>();
    DecodeInto(output, block);

    return output;
}
}  // namespace quicktex::s3tc
