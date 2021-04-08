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
#include "../interpolator/Interpolator.h"
#include "BC1Block.h"

namespace quicktex::s3tc {
class BC1Decoder final : public BlockDecoder<BlockTexture<BC1Block>> {
   public:
    using InterpolatorPtr = std::shared_ptr<Interpolator>;

    BC1Decoder(bool vwrite_alpha, InterpolatorPtr interpolator) : write_alpha(vwrite_alpha), _interpolator(interpolator) {}

    BC1Decoder(bool vwrite_alpha = false) : BC1Decoder(vwrite_alpha, std::make_shared<Interpolator>()) {}

    BC1Decoder(InterpolatorPtr interpolator) : BC1Decoder(false, interpolator) {}

    ColorBlock<4, 4> DecodeBlock(const BC1Block& block) const override;
    ColorBlock<4, 4> DecodeBlock(const BC1Block& block, bool use_3color) const;

    InterpolatorPtr GetInterpolator() const { return _interpolator; }

    bool write_alpha;

   private:
    const InterpolatorPtr _interpolator;
};
}  // namespace quicktex::s3tc
