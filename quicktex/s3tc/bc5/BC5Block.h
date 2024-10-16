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

#include "../bc4/BC4Block.h"

namespace quicktex::s3tc {

class alignas(8) BC5Block {
   public:
    static constexpr int Width = 4;
    static constexpr int Height = 4;

    using BlockPair = std::pair<BC4Block, BC4Block>;

    BC4Block chan0_block;
    BC4Block chan1_block;

    constexpr BC5Block() : chan0_block(BC4Block()), chan1_block(BC4Block()) {
        static_assert(sizeof(BC5Block) == 16);
        static_assert(sizeof(std::array<BC5Block, 10>) == 16 * 10);
        static_assert(alignof(BC5Block) >= 8);
    }

    BC5Block(const BC4Block &chan0, const BC4Block &chan1) {
        chan0_block = chan0;
        chan1_block = chan1;
    }

    BlockPair GetBlocks() const { return BlockPair(chan0_block, chan1_block); }

    void SetBlocks(const BlockPair &pair) {
        chan0_block = pair.first;
        chan1_block = pair.second;
    }

    bool operator==(const BC5Block &Rhs) const { return chan0_block == Rhs.chan0_block && chan1_block == Rhs.chan1_block; }
    bool operator!=(const BC5Block &Rhs) const { return !(Rhs == *this); }
};
}  // namespace quicktex::s3tc