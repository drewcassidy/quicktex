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
#include "Color.h"

#include <algorithm>
#include <stdexcept>

#include "Vector4.h"
#include "Vector4Int.h"
#include "util.h"  // for scale5To8, scale8To5, assert5bit, scale6To8

namespace quicktex {

Color::Color(Vector4Int v) {
    if (v.MaxAbs() > 0xFF) throw std::invalid_argument("Vector members out of range");
    for (int i = 0; i < 4; i++) {
        if (v[i] < 0) throw std::range_error("Color members cannot be negative");
    }

    r = static_cast<uint8_t>(v[0]);
    g = static_cast<uint8_t>(v[1]);
    b = static_cast<uint8_t>(v[2]);
    a = static_cast<uint8_t>(v[3]);
}

uint16_t Color::Pack565Unscaled(uint8_t r, uint8_t g, uint8_t b) {
    assert5bit(r);
    assert6bit(g);
    assert5bit(b);
    return static_cast<uint16_t>(b | (g << 5) | (r << 11));
}

uint16_t Color::Pack565(uint8_t r, uint8_t g, uint8_t b) { return Pack565Unscaled(scale8To5(r), scale8To6(g), scale8To5(b)); }

Color Color::Unpack565Unscaled(uint16_t Packed) {
    uint8_t r = (Packed >> 11) & 0x1F;
    uint8_t g = (Packed >> 5) & 0x3F;
    uint8_t b = Packed & 0x1F;

    return Color(r, g, b);
}

Color Color::Unpack565(uint16_t Packed) {
    uint8_t r = static_cast<uint8_t>(scale5To8((Packed >> 11) & 0x1FU));
    uint8_t g = static_cast<uint8_t>(scale6To8((Packed >> 5) & 0x3FU));
    uint8_t b = static_cast<uint8_t>(scale5To8(Packed & 0x1FU));

    return Color(r, g, b);
}

Color Color::PreciseRound565(Vector4 &v) {
    int trial_r = (int)(v[0] * UINT5_MAX);
    int trial_g = (int)(v[1] * UINT6_MAX);
    int trial_b = (int)(v[2] * UINT5_MAX);

    // clamp to prevent weirdness with slightly out of bounds float values
    uint8_t r = (uint8_t)clampi(trial_r, 0, UINT5_MAX);
    uint8_t g = (uint8_t)clampi(trial_g, 0, UINT6_MAX);
    uint8_t b = (uint8_t)clampi(trial_b, 0, UINT5_MAX);

    // increment each channel if above the rounding point
    r += v[0] > Midpoints5bit[r];
    g += v[1] > Midpoints6bit[g];
    b += v[2] > Midpoints5bit[b];

    assert5bit(r);
    assert6bit(g);
    assert5bit(b);

    return Color(r, g, b);
}

void Color::SetRGB(uint8_t vr, uint8_t vg, uint8_t vb) {
    r = vr;
    g = vg;
    b = vb;
}

size_t Color::MinChannelRGB() {
    if (r <= g && r <= b) return 0;
    if (g <= b && g <= r) return 1;
    return 2;
}

size_t Color::MaxChannelRGB() {
    if (r >= g && r >= b) return 0;
    if (g >= b && g >= r) return 1;
    return 2;
}

Color Color::Min(const Color &A, const Color &B) { return Color(std::min(A[0], B[0]), std::min(A[1], B[1]), std::min(A[2], B[2]), std::min(A[3], B[3])); }

Color Color::Max(const Color &a, const Color &b) { return Color(std::max(a[0], b[0]), std::max(a[1], b[1]), std::max(a[2], b[2]), std::max(a[3], b[3])); }

Color::operator Vector4() const { return Vector4(r, g, b, a); }
Color::operator Vector4Int() const { return Vector4Int(r, g, b, a); }
Vector4Int operator-(const Color &lhs, const Color &rhs) {
    Vector4Int result;
    for (unsigned i = 0; i < 4; i++) { result[i] = (int)lhs[i] - rhs[i]; }
    return result;
}

uint16_t Color::Pack565() const { return Pack565(r, g, b); }
uint16_t Color::Pack565Unscaled() const { return Pack565Unscaled(r, g, b); }

Color Color::ScaleTo565() const { return Color(scale8To5(r), scale8To6(g), scale8To5(b)); }
Color Color::ScaleFrom565() const { return Color(scale5To8(r), scale6To8(g), scale5To8(b)); }

bool Color::operator==(const Color &Rhs) const { return r == Rhs.r && g == Rhs.g && b == Rhs.b && a == Rhs.a; }
bool Color::operator!=(const Color &Rhs) const { return !(Rhs == *this); }

}  // namespace quicktex