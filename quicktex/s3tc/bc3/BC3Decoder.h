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
