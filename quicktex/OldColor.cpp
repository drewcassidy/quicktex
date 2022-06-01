/*  Quicktex Texture Compression Library
    Copyright (C) 2021-2022 Andrew Cassidy <drewcassidy@me.com>
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
#include "OldColor.h"

#include <algorithm>
#include <stdexcept>

#include "Vector4.h"
#include "Vector4Int.h"
#include "util.h"  // for scale_to_8<5>, scale_from_8<5>, assert5bit, scale_to_8<6>

namespace quicktex {

OldColor::OldColor(Vector4Int v) {
    if (v.MaxAbs() > 0xFF) throw std::invalid_argument("Vector members out of range");
    for (int i = 0; i < 4; i++) {
        if (v[i] < 0) throw std::range_error("Color members cannot be negative");
    }

    r = static_cast<uint8_t>(v[0]);
    g = static_cast<uint8_t>(v[1]);
    b = static_cast<uint8_t>(v[2]);
    a = static_cast<uint8_t>(v[3]);
}

uint16_t OldColor::Pack565Unscaled(uint8_t r, uint8_t g, uint8_t b) {
    assert5bit(r);
    assert6bit(g);
    assert5bit(b);
    return static_cast<uint16_t>(b | (g << 5) | (r << 11));
}

uint16_t OldColor::Pack565(uint8_t r, uint8_t g, uint8_t b) {
    return Pack565Unscaled(scale_from_8<5>(r), scale_from_8<6>(g), scale_from_8<5>(b));
}

OldColor OldColor::Unpack565Unscaled(uint16_t Packed) {
    uint8_t r = (Packed >> 11) & 0x1F;
    uint8_t g = (Packed >> 5) & 0x3F;
    uint8_t b = Packed & 0x1F;

    return OldColor(r, g, b);
}

OldColor OldColor::Unpack565(uint16_t Packed) {
    uint8_t r = static_cast<uint8_t>(scale_to_8<5>((Packed >> 11) & 0x1FU));
    uint8_t g = static_cast<uint8_t>(scale_to_8<6>((Packed >> 5) & 0x3FU));
    uint8_t b = static_cast<uint8_t>(scale_to_8<5>(Packed & 0x1FU));

    return OldColor(r, g, b);
}

OldColor OldColor::PreciseRound565(Vector4 &v) {
    int trial_r = (int)(v[0] * UINT5_MAX);
    int trial_g = (int)(v[1] * UINT6_MAX);
    int trial_b = (int)(v[2] * UINT5_MAX);

    // clamp to prevent weirdness with slightly out of bounds float values
    uint8_t r = (uint8_t)clamp<int>(trial_r, 0, UINT5_MAX);
    uint8_t g = (uint8_t)clamp<int>(trial_g, 0, UINT6_MAX);
    uint8_t b = (uint8_t)clamp<int>(trial_b, 0, UINT5_MAX);

    // increment each channel if above the rounding point
    r += v[0] > Midpoints5bit[r];
    g += v[1] > Midpoints6bit[g];
    b += v[2] > Midpoints5bit[b];

    assert5bit(r);
    assert6bit(g);
    assert5bit(b);

    return OldColor(r, g, b);
}

void OldColor::SetRGB(uint8_t vr, uint8_t vg, uint8_t vb) {
    r = vr;
    g = vg;
    b = vb;
}

size_t OldColor::MaxChannelRGB() {
    if (r >= g && r >= b) return 0;
    if (g >= b && g >= r) return 1;
    return 2;
}

OldColor::operator Vector4() const { return Vector4(r, g, b, a); }
OldColor::operator Vector4Int() const { return Vector4Int(r, g, b, a); }
Vector4Int operator-(const OldColor &lhs, const OldColor &rhs) {
    Vector4Int result;
    for (unsigned i = 0; i < 4; i++) { result[i] = (int)lhs[i] - rhs[i]; }
    return result;
}

uint16_t OldColor::Pack565() const { return Pack565(r, g, b); }
uint16_t OldColor::Pack565Unscaled() const { return Pack565Unscaled(r, g, b); }

OldColor OldColor::ScaleTo565() const { return OldColor(scale_from_8<5>(r), scale_from_8<6>(g), scale_from_8<5>(b)); }
OldColor OldColor::ScaleFrom565() const { return OldColor(scale_to_8<5>(r), scale_to_8<6>(g), scale_to_8<5>(b)); }

bool OldColor::operator==(const OldColor &Rhs) const { return r == Rhs.r && g == Rhs.g && b == Rhs.b && a == Rhs.a; }
bool OldColor::operator!=(const OldColor &Rhs) const { return !(Rhs == *this); }

}  // namespace quicktex