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

#include "BC4Decoder.h"

#include <array>    // for array
#include <cassert>  // for assert

#include "../../BlockView.h"  // for ColorBlock
#include "../../formats/blocks/BC4Block.h"
#include "../../ndebug.h"  // for ndebug

void quicktex::BC4Decoder::DecodeBlock(Byte4x4 dest, BC4Block *const block) const noexcept(ndebug) {
    auto l = block->GetLowAlpha();
    auto h = block->GetHighAlpha();

    auto values = BC4Block::GetValues(l, h);
    auto selectors = block->UnpackSelectors();

    for (unsigned y = 0; y < 4; y++) {
        for (unsigned x = 0; x < 4; x++) {
            const auto selector = selectors[y][x];
            assert(selector < 8);
            dest.Set(x, y, values[selector]);
        }
    }
}
