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

#include "../BC1/BC1Decoder.h"
#include "../BC4/BC4Decoder.h"
#include "../BlockDecoder.h"
#include "../blocks.h"
#include "../interpolator.h"
#include "../ndebug.h"

namespace rgbcx {
class BC3Decoder : public BlockDecoder<BC3Block, 4, 4> {
   public:
    BC3Decoder(const Interpolator &interpolator = Interpolator()) : BC3Decoder(BC1Decoder(interpolator, false)) {}
    BC3Decoder(const BC1Decoder &bc1_decoder, const BC4Decoder &bc4_decoder = BC4Decoder()) : _bc1_decoder(bc1_decoder), _bc4_decoder(bc4_decoder) {}

    void DecodeBlock(Color4x4 dest, BC3Block *const block) const noexcept(ndebug) override;

   private:
    const BC1Decoder &_bc1_decoder;
    const BC4Decoder &_bc4_decoder;
};
}  // namespace rgbcx
