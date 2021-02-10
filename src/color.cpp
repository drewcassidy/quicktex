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

#include "color.h"

#include <algorithm>
#include <cassert>

#include "util.h"

// region Color32 implementation
Color32::Color32() { Set(0, 0, 0, 0xFF); }

Color32::Color32(uint8_t R, uint8_t G, uint8_t B, uint8_t A) { Set(R, G, B, A); }

uint16_t Color32::Pack565Unscaled(uint16_t R, uint16_t G, uint16_t B) { return B | (G << 5) | (R << 11); }

uint16_t Color32::Pack565(uint16_t R, uint16_t G, uint16_t B) { return Pack565Unscaled(scale8To5(R), scale8To6(G), scale8To5(B)); }

Color32 Color32::Unpack565(uint16_t Packed) {
    uint8_t R = scale5To8((Packed >> 11) & 0x1F);
    uint8_t G = scale6To8((Packed >> 5) & 0x3F);
    uint8_t B = scale5To8(Packed & 0x1F);

    return Color32(R, G, B);
}

uint8_t Color32::operator[](uint32_t Index) const {
    assert(Index < 4);
    return c[Index];
}

uint8_t &Color32::operator[](uint32_t Index) {
    assert(Index < 4);
    return c[Index];
}

void Color32::Set(uint8_t R, uint8_t G, uint8_t B, uint8_t A) {
    this->r = R;
    this->g = G;
    this->b = B;
    this->a = A;
}

void Color32::Set(const Color32 &Other) {
    this->r = Other.r;
    this->g = Other.g;
    this->b = Other.b;
    this->a = Other.a;
}

Color32 Color32::min(const Color32 &a, const Color32 &b) {
    return Color32(std::min(a[0], b[0]), std::min(a[1], b[1]), std::min(a[2], b[2]), std::min(a[3], b[3]));
}

Color32 Color32::max(const Color32 &a, const Color32 &b) {
    return Color32(std::max(a[0], b[0]), std::max(a[1], b[1]), std::max(a[2], b[2]), std::max(a[3], b[3]));
}

uint16_t Color32::pack565() { return Pack565(r, g, b); }

uint16_t Color32::pack565Unscaled() { return Pack565Unscaled(r, g, b); }
// endregion