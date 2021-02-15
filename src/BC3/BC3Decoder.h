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

#include <memory>

#include "../BC1/BC1Decoder.h"
#include "../BC4/BC4Decoder.h"
#include "../BlockDecoder.h"
#include "../ColorBlock.h"
#include "../interpolator.h"
#include "../ndebug.h"
#include "BC3Block.h"

namespace rgbcx {
class BC3Decoder : public BlockDecoder<BC3Block, 4, 4> {
   public:
    using InterpolatorPtr = std::shared_ptr<Interpolator>;
    using BC1DecoderPtr = std::shared_ptr<BC1Decoder>;
    using BC4DecoderPtr = std::shared_ptr<BC4Decoder>;

    BC3Decoder(InterpolatorPtr interpolator = std::make_shared<Interpolator>()) : BC3Decoder(std::make_shared<BC1Decoder>(interpolator)) {}
    BC3Decoder(BC1DecoderPtr bc1_decoder, BC4DecoderPtr bc4_decoder = std::make_shared<BC4Decoder>()) : _bc1_decoder(bc1_decoder), _bc4_decoder(bc4_decoder) {}

    void DecodeBlock(Color4x4 dest, BC3Block *const block) const noexcept(ndebug) override;

   private:
    const BC1DecoderPtr _bc1_decoder;
    const BC4DecoderPtr _bc4_decoder;
};
}  // namespace rgbcx
