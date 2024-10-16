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
#include "../../Encoder.h"
#include "../../Texture.h"
#include "../bc1/BC1Encoder.h"
#include "../bc4/BC4Encoder.h"
#include "../interpolator/Interpolator.h"
#include "BC3Block.h"

namespace quicktex::s3tc {

class BC3Encoder : public BlockEncoder<BlockTexture<BC3Block>> {
   public:
    using BC1EncoderPtr = std::shared_ptr<BC1Encoder>;
    using BC4EncoderPtr = std::shared_ptr<BC4Encoder>;
    using InterpolatorPtr = std::shared_ptr<Interpolator>;

    BC3Encoder(unsigned level, InterpolatorPtr interpolator)
        : _bc1_encoder(std::make_shared<BC1Encoder>(level, BC1Encoder::ColorMode::FourColor, interpolator)), _bc4_encoder(std::make_shared<BC4Encoder>(3)) {}

    BC3Encoder(unsigned level = 5) : BC3Encoder(level, std::make_shared<Interpolator>()) {}

    BC3Block EncodeBlock(const ColorBlock<4, 4>& pixels) const override;

    BC1EncoderPtr GetBC1Encoder() const { return _bc1_encoder; }
    BC4EncoderPtr GetBC4Encoder() const { return _bc4_encoder; }

   private:
    const BC1EncoderPtr _bc1_encoder;
    const BC4EncoderPtr _bc4_encoder;
};
}  // namespace quicktex::s3tc
