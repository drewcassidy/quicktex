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

#include "BC5Encoder.h"

#include "../../ColorBlock.h"
#include "../bc4/BC4Block.h"

namespace quicktex::s3tc {
BC5Block BC5Encoder::EncodeBlock(const ColorBlock<4, 4> &pixels) const {
    auto output = BC5Block();
    output.chan0_block = _chan0_encoder->EncodeBlock(pixels);
    output.chan1_block = _chan1_encoder->EncodeBlock(pixels);
    return output;
}
}  // namespace quicktex::s3tc