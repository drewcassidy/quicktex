/*  Python-rgbcx Texture Compression Library
    Copyright (C) 2021 Andrew Cassidy <drewcassidy@me.com>
    Partially derived from rgbcx.h written by Richard Geldreich 2020 <richgel99@gmail.com>
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

#include "blocks.h"
#include <algorithm>
#include <cassert>

// region Color32 implementation
Color32::Color32(uint32_t vr, uint32_t vg, uint32_t vb, uint32_t va) { set(vr, vg, vb, va); }

uint8_t Color32::operator[](uint32_t idx) const {
    assert(idx < 4);
    return C[idx];
}

uint8_t &Color32::operator[](uint32_t idx) {
    assert(idx < 4);
    return C[idx];
}

void Color32::set(uint8_t vr, uint8_t vg, uint8_t vb, uint8_t va) {
    C[0] = vr;
    C[1] = vg;
    C[2] = vb;
    C[3] = va;
}

void Color32::set(const Color32 &other) {
    C[0] = other.C[0];
    C[1] = other.C[1];
    C[2] = other.C[2];
}
Color32 Color32::min(const Color32 &a, const Color32 &b) {
    return Color32(std::min(a[0], b[0]), std::min(a[1], b[1]), std::min(a[2], b[2]), std::min(a[3], b[3]));
}
Color32 Color32::max(const Color32 &a, const Color32 &b) {
    return Color32(std::max(a[0], b[0]), std::max(a[1], b[1]), std::max(a[2], b[2]), std::max(a[3], b[3]));
}
// endregion
