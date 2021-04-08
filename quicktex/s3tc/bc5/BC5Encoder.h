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
