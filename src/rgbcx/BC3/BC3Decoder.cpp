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

#include "BC3Decoder.h"

#include <type_traits>

#include "../BlockView.h"
#include "../ndebug.h"
#include "BC3Block.h"

namespace rgbcx {

void BC3Decoder::DecodeBlock(Color4x4 dest, BC3Block *const block) const noexcept(ndebug) {
    _bc1_decoder->DecodeBlock(dest, &(block->color_block));
    _bc4_decoder->DecodeBlock(dest, &(block->alpha_block), 3);
}
}  // namespace rgbcx