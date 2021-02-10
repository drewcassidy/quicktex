/*  Python-rgbcx Texture Compression Library
    Copyright (C) 2021 Andrew Cassidy <drewcassidy@me.com>
    Partially derived from rgbcx.h written by Richard Geldreich 2020 <richgel99@gmail.com>
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

#include "BC1Decoder.h"

#include <array>

#include "ColorBlock.h"

void rgbcx::BC1Decoder::DecodeBlock(const Color4x4 *dest, const BC1Block *block) {
    const unsigned l = block->GetLowColor();
    const unsigned h = block->GetHighColor();

    const auto l_color = Color32::Unpack565(l);
    const auto h_color = Color32::Unpack565(h);

    std::array<Color32, 4> colors;
    colors[0] = l_color;
    colors[1] = h_color;

    bool three_color = (h >= l);
    if (three_color) {
        colors[2] = _interpolator.InterpolateHalfColor(l_color, h_color);
        colors[3] = Color32(0,0,0);
    } else {
        colors[2] = _interpolator.InterpolateColor()
    }
}
