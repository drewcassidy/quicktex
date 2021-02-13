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
class Color {
   private:
    std::array<uint8_t, 4> _channels;

   public:
    Color();

    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xFF);

    static uint16_t Pack565Unscaled(uint8_t r, uint8_t g, uint8_t b);
    static uint16_t Pack565(uint8_t r, uint8_t g, uint8_t b);

    static Color Unpack565Unscaled(uint16_t Packed);
    static Color Unpack565(uint16_t Packed);

    bool operator==(const Color &Rhs) const { return R() == Rhs.R() && G() == Rhs.G() && B() == Rhs.B() && A() == Rhs.A(); }

    uint8_t operator[](size_t index) const {
        assert(index < 4);
        return _channels[index];
    }
    uint8_t &operator[](size_t index) {
        assert(index < 4);
        return _channels[index];
    }

    // more readable versions of index operator for each channel
    uint8_t &R() { return _channels[0]; }
    uint8_t &G() { return _channels[1]; }
    uint8_t &B() { return _channels[2]; }
    uint8_t &A() { return _channels[3]; }

    uint8_t R() const { return _channels[0]; }
    uint8_t G() const { return _channels[1]; }
    uint8_t B() const { return _channels[2]; }
    uint8_t A() const { return _channels[3]; }

    // Assignment functions
    void SetR(uint8_t r) { _channels[0] = r; }
    void SetG(uint8_t g) { _channels[1] = g; }
    void SetB(uint8_t b) { _channels[2] = b; }
    void SetA(uint8_t a) { _channels[3] = a; }

    void SetRGBA(uint8_t vr, uint8_t vg, uint8_t vb, uint8_t va);
    void SetRGBA(const Color &other) { SetRGBA(other.R(), other.G(), other.B(), other.A()); }

    void SetRGB(uint8_t vr, uint8_t vg, uint8_t vb);
    void SetRGB(const Color &other) { SetRGB(other.R(), other.G(), other.B()); }

    uint16_t pack565();
    uint16_t pack565Unscaled();

    Color ScaleTo565() const;
    Color ScaleFrom565() const;

    static Color min(const Color &A, const Color &B);
    static Color max(const Color &A, const Color &B);

    unsigned get_luma() const { return (13938U * R() + 46869U * G() + 4729U * B() + 32768U) >> 16U; }  // REC709 weightings
};
#pragma pack(pop)