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

#include "BC3Decoder.h"

#include "../../ColorBlock.h"
#include "BC3Block.h"

namespace quicktex::s3tc {

ColorBlock<4, 4> BC3Decoder::DecodeBlock(const BC3Block &block) const {
    auto output = _bc1_decoder->DecodeBlock(block.color_block, false);

    _bc4_decoder->DecodeInto(output, block.alpha_block);

    return output;
}
}  // namespace quicktex::s3tc