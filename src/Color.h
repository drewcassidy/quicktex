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

#pragma once
#include <assert.h>  // for assert
#include <stddef.h>  // for size_t

#include <cstdint>  // for uint8_t, uint16_t

#pragma pack(push, 1)
class Color {
   public:
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;

    Color();

    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xFF);

    static uint16_t Pack565Unscaled(uint8_t r, uint8_t g, uint8_t b);
    static uint16_t Pack565(uint8_t r, uint8_t g, uint8_t b);

    static Color Unpack565Unscaled(uint16_t Packed);
    static Color Unpack565(uint16_t Packed);

    bool operator==(const Color &Rhs) const { return r == Rhs.r && g == Rhs.g && b == Rhs.b && a == Rhs.a; }

    uint8_t operator[](size_t index) const {
        assert(index < 4);
        return reinterpret_cast<const uint8_t *>(this)[index];
    }
    uint8_t &operator[](size_t index) {
        assert(index < 4);
        return reinterpret_cast<uint8_t *>(this)[index];
    }

    void SetRGBA(uint8_t vr, uint8_t vg, uint8_t vb, uint8_t va);
    void SetRGBA(const Color &other) { SetRGBA(other.r, other.g, other.b, other.a); }

    void SetRGB(uint8_t vr, uint8_t vg, uint8_t vb);
    void SetRGB(const Color &other) { SetRGB(other.r, other.g, other.b); }

    uint16_t pack565();
    uint16_t pack565Unscaled();

    Color ScaleTo565() const;
    Color ScaleFrom565() const;

    static Color min(const Color &A, const Color &B);
    static Color max(const Color &A, const Color &B);

    int get_luma() const { return (13938U * r + 46869U * g + 4729U * b + 32768U) >> 16U; }  // REC709 weightings
};
#pragma pack(pop)