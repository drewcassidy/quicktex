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

#include <utility>

#include "../bc1/BC1Block.h"
#include "../bc4/BC4Block.h"

namespace quicktex::s3tc {

class alignas(8) BC3Block {
   public:
    static constexpr int Width = 4;
    static constexpr int Height = 4;

    using BlockPair = std::pair<BC4Block, BC1Block>;

    BC4Block alpha_block;
    BC1Block color_block;

    constexpr BC3Block() : alpha_block(BC4Block()), color_block(BC1Block()) {
        static_assert(sizeof(BC3Block) == 16);
        static_assert(sizeof(std::array<BC3Block, 10>) == 16 * 10);
        static_assert(alignof(BC3Block) >= 8);
    }

    BC3Block(const BC4Block &alpha, const BC1Block &color) {
        alpha_block = alpha;
        color_block = color;
    }

    BlockPair GetBlocks() const { return BlockPair(alpha_block, color_block); }

    void SetBlocks(const BlockPair &blocks) {
        alpha_block = blocks.first;
        color_block = blocks.second;
    }

    bool operator==(const BC3Block &Rhs) const { return alpha_block == Rhs.alpha_block && color_block == Rhs.color_block; }
    bool operator!=(const BC3Block &Rhs) const { return !(Rhs == *this); }
};
}  // namespace quicktex::s3tc