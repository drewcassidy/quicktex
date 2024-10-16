/*  Quicktex Texture Compression Library
    Copyright (C) 2021-2024 Andrew Cassidy <drewcassidy@me.com>
    Partially derived from rgbcx.h written by Richard Geldreich <richgel99@gmail.com>
    and licenced under the public domain

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
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
