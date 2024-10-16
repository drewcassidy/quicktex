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
