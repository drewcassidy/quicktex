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

#include "BC5Decoder.h"

#include "../../ColorBlock.h"
#include "BC5Block.h"

namespace quicktex::s3tc {
ColorBlock<4, 4> BC5Decoder::DecodeBlock(const BC5Block &block) const {
    auto output = ColorBlock<4, 4>();
    _chan0_decoder->DecodeInto(output, block.chan0_block);
    _chan1_decoder->DecodeInto(output, block.chan1_block);
    return output;
}
}  // namespace quicktex::s3tc