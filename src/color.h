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

#pragma once
#include <cstdint>

#pragma pack(push, 1)
class Color32 {
   public:
    union {
        struct {
            uint8_t R;
            uint8_t G;
            uint8_t B;
            uint8_t A;
        };

        uint8_t C[4];
    };

    Color32();

    Color32(uint8_t R, uint8_t G, uint8_t B, uint8_t A = 0xFF);

    static uint16_t pack565Unscaled(uint16_t R, uint16_t G, uint16_t B);
    static uint16_t pack565(uint16_t R, uint16_t G, uint16_t B);

    static Color32 unpack565(uint16_t Packed);

    bool operator==(const Color32 &Rhs) const { return R == Rhs.R && G == Rhs.G && B == Rhs.B && A == Rhs.A; }

    uint8_t operator[](uint32_t Index) const;
    uint8_t &operator[](uint32_t Index);

    uint16_t pack565();
    uint16_t pack565Unscaled();

    static Color32 min(const Color32 &A, const Color32 &B);
    static Color32 max(const Color32 &A, const Color32 &B);

    void set(uint8_t R, uint8_t G, uint8_t B, uint8_t A);

    void set(const Color32 &Other);
};
#pragma pack(pop)