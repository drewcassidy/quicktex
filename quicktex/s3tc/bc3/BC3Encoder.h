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

#include "../../BlockEncoder.h"
#include "../../BlockView.h"
#include "../bc1/BC1Encoder.h"
#include "../bc4/BC4Encoder.h"
#include "../interpolator/Interpolator.h"
#include "BC3Block.h"

namespace quicktex::s3tc {

class BC3Encoder : public BlockEncoderTemplate<BC3Block, 4, 4> {
   public:
    using BC1EncoderPtr = std::shared_ptr<BC1Encoder>;
    using BC4EncoderPtr = std::shared_ptr<BC4Encoder>;
    using InterpolatorPtr = std::shared_ptr<Interpolator>;

    BC3Encoder(BC1EncoderPtr bc1_encoder) : _bc1_encoder(bc1_encoder), _bc4_encoder(std::make_shared<BC4Encoder>(3)) {}

    BC3Encoder(unsigned level, bool allow_3color, bool allow_3color_black, InterpolatorPtr interpolator)
        : BC3Encoder(std::make_shared<BC1Encoder>(level, allow_3color, allow_3color_black, interpolator)) {}

    BC3Encoder(unsigned level = 5, bool allow_3color = true, bool allow_3color_black = true)
        : BC3Encoder(std::make_shared<BC1Encoder>(level, allow_3color, allow_3color_black, std::make_shared<Interpolator>())) {}

    void EncodeBlock(Color4x4 pixels, BC3Block *dest) const override;

    BC1EncoderPtr GetBC1Encoder() const { return _bc1_encoder; }
    BC4EncoderPtr GetBC4Encoder() const { return _bc4_encoder; }

   private:
    const BC1EncoderPtr _bc1_encoder;
    const BC4EncoderPtr _bc4_encoder;
};
}  // namespace quicktex::s3tc
