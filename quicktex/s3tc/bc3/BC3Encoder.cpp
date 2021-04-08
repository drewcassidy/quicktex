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

#include "BC3Encoder.h"

#include "../../ColorBlock.h"
#include "../bc1/BC1Block.h"
#include "../bc4/BC4Block.h"
#include "BC3Block.h"

namespace quicktex::s3tc {
BC3Block BC3Encoder::EncodeBlock(const ColorBlock<4, 4> &pixels) const {
    auto output = BC3Block();
    output.color_block = _bc1_encoder->EncodeBlock(pixels);
    output.alpha_block = _bc4_encoder->EncodeBlock(pixels);
    return output;
}
}  // namespace quicktex::s3tc