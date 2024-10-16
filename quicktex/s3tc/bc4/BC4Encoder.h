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

#include <cstdint>
#include <stdexcept>

#include "../../ColorBlock.h"
#include "../../Encoder.h"
#include "../../Texture.h"
#include "BC4Block.h"

namespace quicktex::s3tc {

class BC4Encoder : public BlockEncoder<BlockTexture<BC4Block>> {
   public:
    BC4Encoder(const uint8_t channel) {
        if (channel >= 4) throw std::invalid_argument("Channel out of range");
        _channel = channel;
    }

    BC4Block EncodeBlock(const ColorBlock<4, 4> &pixels) const override;

    uint8_t GetChannel() const { return _channel; }

   private:
    uint8_t _channel;
};
}  // namespace quicktex::s3tc
