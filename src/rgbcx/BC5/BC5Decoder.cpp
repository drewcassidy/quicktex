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

#include "BC5Decoder.h"

#include "../BlockView.h"
#include "../ndebug.h"
#include "BC5Block.h"

namespace rgbcx {

void BC5Decoder::DecodeBlock(Color4x4 dest, BC5Block *const block) const noexcept(ndebug) {
    _chan0_decoder->DecodeBlock(dest, &block->chan0_block);
    _chan1_decoder->DecodeBlock(dest, &block->chan1_block);
}
}  // namespace rgbcx