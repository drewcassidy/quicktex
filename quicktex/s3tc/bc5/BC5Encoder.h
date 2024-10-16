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
#include <memory>
#include <tuple>
#include <type_traits>

#include "../../ColorBlock.h"
#include "../../Encoder.h"
#include "../../Texture.h"
#include "../bc4/BC4Encoder.h"
#include "BC5Block.h"

namespace quicktex::s3tc {
class BC5Encoder : public BlockEncoder<BlockTexture<BC5Block>> {
   public:
    using ChannelPair = std::tuple<uint8_t, uint8_t>;
    using BC4EncoderPtr = std::shared_ptr<BC4Encoder>;
    using BC4EncoderPair = std::tuple<BC4EncoderPtr, BC4EncoderPtr>;

    BC5Encoder(uint8_t chan0 = 0, uint8_t chan1 = 1) : BC5Encoder(std::make_shared<BC4Encoder>(chan0), std::make_shared<BC4Encoder>(chan1)) {}
    BC5Encoder(BC4EncoderPtr chan0_encoder, BC4EncoderPtr chan1_encoder) : _chan0_encoder(chan0_encoder), _chan1_encoder(chan1_encoder) {}

    BC5Block EncodeBlock(const ColorBlock<4, 4> &pixels) const override;

    ChannelPair GetChannels() const { return ChannelPair(_chan0_encoder->GetChannel(), _chan1_encoder->GetChannel()); }

    BC4EncoderPair GetBC4Encoders() const { return BC4EncoderPair(_chan0_encoder, _chan1_encoder); }

   private:
    const BC4EncoderPtr _chan0_encoder;
    const BC4EncoderPtr _chan1_encoder;
};
}  // namespace quicktex::s3tc
