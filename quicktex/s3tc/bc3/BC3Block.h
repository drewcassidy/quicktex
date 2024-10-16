/*  Quicktex Texture Compression Library
    Copyright (C) 2021-2024 Andrew Cassidy <drewcassidy@me.com>
    Partially derived from rgbcx.h written by Richard Geldreich <richgel99@gmail.com>
    and licenced under the public domain

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
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