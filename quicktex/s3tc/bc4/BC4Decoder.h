/*  Quicktex Texture Compression Library
    Copyright (C) 2021-2022 Andrew Cassidy <drewcassidy@me.com>
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
#include <stdexcept>

#include "../../ColorBlock.h"
#include "../../Decoder.h"
#include "../../Texture.h"
#include "BC4Block.h"

namespace quicktex::s3tc {

class BC4Decoder : public BlockDecoder<BlockTexture<BC4Block>> {
   public:
    BC4Decoder(uint8_t channel = 3) {
        if (channel >= 4U) throw std::invalid_argument("Channel out of range");
        _channel = channel;
    }

    ColorBlock<4, 4> DecodeBlock(const BC4Block &block) const override;

    void DecodeInto(ColorBlock<4, 4> &dest, const BC4Block &block) const;

    uint8_t GetChannel() const { return _channel; }

   private:
    uint8_t _channel;
};
}  // namespace quicktex::s3tc
