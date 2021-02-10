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
#include <array>
#include <cstdint>

#pragma pack(push, 1)
class Color32 {
   public:
    union {
        struct {
            uint8_t r;
            uint8_t g;
            uint8_t b;
            uint8_t a;
        };

        std::array<uint8_t, 4> c;
    };

    Color32();

    Color32(uint8_t R, uint8_t G, uint8_t B, uint8_t A = 0xFF);

    static uint16_t Pack565Unscaled(uint16_t R, uint16_t G, uint16_t B);
    static uint16_t Pack565(uint16_t R, uint16_t G, uint16_t B);

    static Color32 Unpack565Unscaled(uint16_t Packed);
    static Color32 Unpack565(uint16_t Packed);

    bool operator==(const Color32 &Rhs) const { return r == Rhs.r && g == Rhs.g && b == Rhs.b && a == Rhs.a; }

    uint8_t operator[](uint32_t Index) const;
    uint8_t &operator[](uint32_t Index);

    uint16_t pack565();
    uint16_t pack565Unscaled();

    Color32 ScaleTo565() const;
    Color32 ScaleFrom565() const;

    static Color32 min(const Color32 &A, const Color32 &B);
    static Color32 max(const Color32 &A, const Color32 &B);

    void Set(uint8_t vr, uint8_t vg, uint8_t vb, uint8_t va);
    void Set(const Color32 &other) { Set(other.r, other.g, other.b, other.a); }

    void SetRGB(uint8_t vr, uint8_t vg, uint8_t vb);
    void SetRGB(const Color32 &other) { SetRGB(other.r, other.g, other.b); }
};
#pragma pack(pop)