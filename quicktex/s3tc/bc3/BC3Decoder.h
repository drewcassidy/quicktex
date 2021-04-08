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

#include <memory>

#include "../../ColorBlock.h"
#include "../../Decoder.h"
#include "../../Texture.h"
#include "../bc1/BC1Decoder.h"
#include "../bc4/BC4Decoder.h"
#include "../interpolator/Interpolator.h"
#include "BC3Block.h"

namespace quicktex::s3tc {

class BC3Decoder : public BlockDecoder<BlockTexture<BC3Block>> {
   public:
    using BC1DecoderPtr = std::shared_ptr<BC1Decoder>;
    using BC4DecoderPtr = std::shared_ptr<BC4Decoder>;
    using InterpolatorPtr = std::shared_ptr<Interpolator>;

    BC3Decoder(InterpolatorPtr interpolator) : _bc1_decoder(std::make_shared<BC1Decoder>(interpolator)), _bc4_decoder(std::make_shared<BC4Decoder>(3)) {}

    BC3Decoder() : BC3Decoder(std::make_shared<Interpolator>()) {}

    ColorBlock<4, 4> DecodeBlock(const BC3Block &block) const override;

    BC1DecoderPtr GetBC1Decoder() const { return _bc1_decoder; }
    BC4DecoderPtr GetBC4Decoder() const { return _bc4_decoder; }

   private:
    const BC1DecoderPtr _bc1_decoder;
    const BC4DecoderPtr _bc4_decoder;
};
}  // namespace quicktex::s3tc
