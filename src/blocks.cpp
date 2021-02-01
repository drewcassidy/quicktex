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

// region color32 implementation
color32::color32(uint32_t vr, uint32_t vg, uint32_t vb, uint32_t va) { set(vr, vg, vb, va); }

uint8_t color32::operator[](uint32_t idx) const {
    assert(idx < 4);
    return c[idx];
}

uint8_t &color32::operator[](uint32_t idx) {
    assert(idx < 4);
    return c[idx];
}

void color32::set(uint8_t vr, uint8_t vg, uint8_t vb, uint8_t va) {
    c[0] = vr;
    c[1] = vg;
    c[2] = vb;
    c[3] = va;
}

void color32::set_rgb(const color32 &other) {
    c[0] = other.c[0];
    c[1] = other.c[1];
    c[2] = other.c[2];
}
color32 color32::comp_min(const color32 &a, const color32 &b) {
    return color32(std::min(a[0], b[0]), std::min(a[1], b[1]), std::min(a[2], b[2]), std::min(a[3], b[3]));
}
color32 color32::comp_max(const color32 &a, const color32 &b) {
    return color32(std::max(a[0], b[0]), std::max(a[1], b[1]), std::max(a[2], b[2]), std::max(a[3], b[3]));
}
// endregion
