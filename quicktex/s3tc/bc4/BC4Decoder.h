/*  Python-rgbcx Texture Compression Library
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

#include <cassert>
#include <cstdint>
#include <stdexcept>

#include "../../BlockDecoder.h"
#include "../../BlockView.h"
#include "../../ndebug.h"
#include "BC4Block.h"

namespace quicktex {
class BC4Decoder : public BlockDecoderTemplate<BC4Block, 4, 4> {
   public:
    BC4Decoder(uint8_t channel = 3) { SetChannel(channel); }

    void DecodeBlock(Color4x4 dest, BC4Block *const block) const noexcept(ndebug) override { DecodeBlock(dest.GetChannel(_channel), block); }
    void DecodeBlock(Color4x4 dest, BC4Block *const block, uint8_t channel) const noexcept(ndebug) { DecodeBlock(dest.GetChannel(channel), block); }
    void DecodeBlock(Byte4x4 dest, BC4Block *const block) const noexcept(ndebug);

    uint8_t GetChannel() const { return _channel; }
    void SetChannel(uint8_t channel) {
        if (channel >= 4U) throw std::invalid_argument("Channel out of range");
        _channel = channel;
    }

   private:
    uint8_t _channel;
};
}  // namespace quicktex
