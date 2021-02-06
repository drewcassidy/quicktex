/*  Python-rgbcx Texture Compression Library
    Copyright (C) 2021 Andrew Cassidy <drewcassidy@me.com>
    Partially derived from rgbcx.h written by Richard Geldreich <richgel99@gmail.com>
    and licenced under the public domain

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "color.h"
#include <algorithm>
#include <cassert>

// region Color32 implementation
Color32::Color32() { set(0, 0, 0, 0xFF); }

Color32::Color32(uint8_t R, uint8_t G, uint8_t B, uint8_t A) { set(R, G, B, A); }

uint16_t Color32::pack565Unscaled(uint16_t R, uint16_t G, uint16_t B) { return B | (G << 5) | (R << 11); }

uint16_t Color32::pack565(uint16_t R, uint16_t G, uint16_t B) { return pack565Unscaled(scale8To5(R), scale8To6(G), scale8To5(B)); }

Color32 Color32::unpack565(uint16_t Packed) {
    uint8_t R = scale5To8((Packed >> 11) & 0x1F);
    uint8_t G = scale6To8((Packed >> 5) & 0x3F);
    uint8_t B = scale5To8(Packed & 0x1F);

    return Color32(R, G, B);
}

uint8_t Color32::operator[](uint32_t Index) const {
    assert(Index < 4);
    return C[Index];
}

uint8_t &Color32::operator[](uint32_t Index) {
    assert(Index < 4);
    return C[Index];
}

void Color32::set(uint8_t R, uint8_t G, uint8_t B, uint8_t A) {
    this->R = R;
    this->G = G;
    this->B = B;
    this->A = A;
}

void Color32::set(const Color32 &Other) {
    this->R = Other.R;
    this->G = Other.G;
    this->B = Other.B;
    this->A = Other.A;
}

Color32 Color32::min(const Color32 &a, const Color32 &b) {
    return Color32(std::min(a[0], b[0]), std::min(a[1], b[1]), std::min(a[2], b[2]), std::min(a[3], b[3]));
}

Color32 Color32::max(const Color32 &a, const Color32 &b) {
    return Color32(std::max(a[0], b[0]), std::max(a[1], b[1]), std::max(a[2], b[2]), std::max(a[3], b[3]));
}

uint16_t Color32::pack565() { return pack565(R, G, B); }

uint16_t Color32::pack565Unscaled() { return pack565Unscaled(R, G, B); }
// endregion