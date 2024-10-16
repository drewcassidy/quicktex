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
#include "../../Decoder.h"
#include "../../Texture.h"
#include "../bc4/BC4Decoder.h"
#include "BC5Block.h"

namespace quicktex::s3tc {

class BC5Decoder : public BlockDecoder<BlockTexture<BC5Block>> {
   public:
    using ChannelPair = std::tuple<uint8_t, uint8_t>;
    using BC4DecoderPtr = std::shared_ptr<BC4Decoder>;
    using BC4DecoderPair = std::tuple<BC4DecoderPtr, BC4DecoderPtr>;

    BC5Decoder(uint8_t chan0 = 0, uint8_t chan1 = 1) : BC5Decoder(std::make_shared<BC4Decoder>(chan0), std::make_shared<BC4Decoder>(chan1)) {}
    BC5Decoder(BC4DecoderPtr chan0_decoder, BC4DecoderPtr chan1_decoder) : _chan0_decoder(chan0_decoder), _chan1_decoder(chan1_decoder) {}

    ColorBlock<4, 4> DecodeBlock(const BC5Block &block) const override;

    ChannelPair GetChannels() const { return ChannelPair(_chan0_decoder->GetChannel(), _chan1_decoder->GetChannel()); }

    BC4DecoderPair GetBC4Decoders() const { return BC4DecoderPair(_chan0_decoder, _chan1_decoder); }

   private:
    const BC4DecoderPtr _chan0_decoder;
    const BC4DecoderPtr _chan1_decoder;
};
}  // namespace quicktex::s3tc
