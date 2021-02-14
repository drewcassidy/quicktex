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

#include <stddef.h>

#include "../BlockDecoder.h"
#include "../ColorBlock.h"
#include "../blocks.h"
#include "../ndebug.h"

namespace rgbcx {
class BC4Decoder : public BlockDecoder<BC4Block, 4, 4> {
   public:
    BC4Decoder(size_t channel = 3) : _channel(channel) {}

    void DecodeBlock(Color4x4 dest, BC4Block *const block) const noexcept(ndebug) override { DecodeBlock(dest, block, _channel); }
    void DecodeBlock(Color4x4 dest, BC4Block *const block, size_t channel) const noexcept(ndebug);

    constexpr size_t GetChannel() const { return _channel; }

   private:
    const size_t _channel;
};
}  // namespace rgbcx