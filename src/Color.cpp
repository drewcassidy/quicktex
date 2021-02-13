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

#include "Color.h"

#include <algorithm>
#include <cassert>

#include "util.h"

// region Color implementation
Color::Color() { SetRGBA(0, 0, 0, 0xFF); }

Color::Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) { SetRGBA(r, g, b, a); }

uint16_t Color::Pack565Unscaled(uint8_t r, uint8_t g, uint8_t b) {
    Assert5Bit(r);
    Assert6Bit(g);
    Assert5Bit(b);
    return static_cast<uint16_t>(b | (g << 5) | (r << 11));
}

uint16_t Color::Pack565(uint8_t r, uint8_t g, uint8_t b) { return Pack565Unscaled(scale8To5(r), scale8To6(g), scale8To5(b)); }

Color Color::Unpack565(uint16_t Packed) {
    uint8_t r = static_cast<uint8_t>(scale5To8((Packed >> 11) & 0x1FU));
    uint8_t g = static_cast<uint8_t>(scale6To8((Packed >> 5) & 0x3FU));
    uint8_t b = static_cast<uint8_t>(scale5To8(Packed & 0x1FU));

    return Color(r, g, b);
}

Color Color::Unpack565Unscaled(uint16_t Packed) {
    uint8_t r = (Packed >> 11) & 0x1F;
    uint8_t g = (Packed >> 5) & 0x3F;
    uint8_t b = Packed & 0x1F;

    return Color(r, g, b);
}

void Color::SetRGBA(uint8_t vr, uint8_t vg, uint8_t vb, uint8_t va = 0xFF) {
    r = vr;
    g = vg;
    b = vb;
    a = va;
}

void Color::SetRGB(uint8_t vr, uint8_t vg, uint8_t vb) {
    r = vr;
    g = vg;
    b = vb;
}

Color Color::min(const Color &a, const Color &b) { return Color(std::min(a[0], b[0]), std::min(a[1], b[1]), std::min(a[2], b[2]), std::min(a[3], b[3])); }

Color Color::max(const Color &a, const Color &b) { return Color(std::max(a[0], b[0]), std::max(a[1], b[1]), std::max(a[2], b[2]), std::max(a[3], b[3])); }

uint16_t Color::pack565() { return Pack565(r, g, b); }

uint16_t Color::pack565Unscaled() { return Pack565Unscaled(r, g, b); }

Color Color::ScaleTo565() const { return Color(scale8To5(r), scale8To6(g), scale8To5(b)); }
Color Color::ScaleFrom565() const { return Color(scale5To8(r), scale6To8(g), scale5To8(b)); }

// endregion