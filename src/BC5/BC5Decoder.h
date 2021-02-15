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
#include <memory>

#include "../BC4/BC4Decoder.h"
#include "../BlockDecoder.h"
#include "../ColorBlock.h"
#include "../blocks.h"
#include "../ndebug.h"

namespace rgbcx {
class BC5Decoder : public BlockDecoder<BC5Block, 4, 4> {
   public:
    using BC4DecoderPtr = std::shared_ptr<BC4Decoder>;

    BC5Decoder(size_t chan0 = 0, size_t chan1 = 1) : BC5Decoder(std::make_shared<BC4Decoder>(), chan0, chan1) {}
    BC5Decoder(BC4DecoderPtr bc4_decoder, size_t chan0 = 0, size_t chan1 = 1) : _bc4_decoder(bc4_decoder), _chan0(chan0), _chan1(chan1) {}

    void DecodeBlock(Color4x4 dest, BC5Block *const block) const noexcept(ndebug) override;

    constexpr size_t GetChannel0() const { return _chan0; }
    constexpr size_t GetChannel1() const { return _chan1; }

   private:
    const BC4DecoderPtr _bc4_decoder;
    const size_t _chan0;
    const size_t _chan1;
};
}  // namespace rgbcx
