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
