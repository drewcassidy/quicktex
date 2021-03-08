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

#include "../BlockDecoder.h"
#include "../BlockView.h"
#include "../Interpolator.h"
#include "../ndebug.h"
#include "BC1Block.h"

namespace rgbcx {
class BC1Decoder final : public BlockDecoder<BC1Block, 4, 4> {
   public:
    using InterpolatorPtr = std::shared_ptr<Interpolator>;
    BC1Decoder(const InterpolatorPtr interpolator = std::make_shared<Interpolator>(), bool write_alpha = false)
        : _interpolator(interpolator), _write_alpha(write_alpha) {}

    void DecodeBlock(Color4x4 dest, BC1Block *const block) const noexcept(ndebug) override;

    InterpolatorPtr GetInterpolator() const { return _interpolator; }
    constexpr bool WritesAlpha() const { return _write_alpha; }

   private:
    const InterpolatorPtr _interpolator;
    const bool _write_alpha;
};
}  // namespace rgbcx
