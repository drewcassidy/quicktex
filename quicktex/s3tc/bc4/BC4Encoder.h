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
