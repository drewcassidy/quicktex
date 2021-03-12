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

#include "../BlockEncoder.h"
#include "../BlockView.h"
#include "../ndebug.h"
#include "BC4Block.h"

namespace rgbcx {

class BC4Encoder : public BlockEncoderTemplate<BC4Block, 4, 4> {
   public:
    BC4Encoder(const uint8_t channel) { SetChannel(channel); }

    void EncodeBlock(Color4x4 pixels, BC4Block *const dest) const override { EncodeBlock(pixels.GetChannel(_channel), dest); }
    void EncodeBlock(Color4x4 pixels, BC4Block *const dest, uint8_t channel) const noexcept(ndebug) { EncodeBlock(pixels.GetChannel(channel), dest); }
    void EncodeBlock(Byte4x4 pixels, BC4Block *const dest) const noexcept(ndebug);

    uint8_t GetChannel() const { return _channel; }
    void SetChannel(uint8_t channel) {
        if (channel >= 4) throw std::invalid_argument("Channel out of range");
        _channel = channel;
    }

   private:
    uint8_t _channel;
};
}  // namespace rgbcx
