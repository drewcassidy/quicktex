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
