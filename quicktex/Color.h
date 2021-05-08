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

#pragma once
#include <cassert>  // for assert
#include <cstddef>  // for size_t
#include <cstdint>  // for uint8_t, uint16_t

namespace quicktex {
class Vector4;
class Vector4Int;

#pragma pack(push, 1)
class Color {
   public:
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;

    constexpr Color() : Color(0, 0, 0, 0xFF) {}

    constexpr Color(uint8_t vr, uint8_t vg, uint8_t vb, uint8_t va = 0xFF) : r(vr), g(vg), b(vb), a(va) {}

    Color(Vector4Int v);

    static uint16_t Pack565Unscaled(uint8_t r, uint8_t g, uint8_t b);
    static uint16_t Pack565(uint8_t r, uint8_t g, uint8_t b);

    static Color Unpack565Unscaled(uint16_t Packed);
    static Color Unpack565(uint16_t Packed);

    static Color PreciseRound565(Vector4 &v);

    static Color Min(const Color &A, const Color &B);
    static Color Max(const Color &A, const Color &B);

    bool operator==(const Color &Rhs) const;
    bool operator!=(const Color &Rhs) const;

    uint8_t operator[](size_t index) const {
        assert(index < 4);
        return reinterpret_cast<const uint8_t *>(this)[index];
    }
    uint8_t &operator[](size_t index) {
        assert(index < 4);
        return reinterpret_cast<uint8_t *>(this)[index];
    }

    operator Vector4() const;
    operator Vector4Int() const;
    friend Vector4Int operator-(const Color &lhs, const Color &rhs);

    void SetRGB(uint8_t vr, uint8_t vg, uint8_t vb);
    void SetRGB(const Color &other) { SetRGB(other.r, other.g, other.b); }

    uint16_t Pack565() const;
    uint16_t Pack565Unscaled() const;

    Color ScaleTo565() const;
    Color ScaleFrom565() const;

    size_t MinChannelRGB();
    size_t MaxChannelRGB();

    bool IsGrayscale() const { return ((r == g) && (r == b)); }
    bool IsBlack() const { return (r | g | b) < 4; }

    int GetLuma() const { return (13938U * r + 46869U * g + 4729U * b + 32768U) >> 16U; }  // REC709 weightings

   private:
    static constexpr float Midpoints5bit[32] = {.015686f, .047059f, .078431f, .111765f, .145098f, .176471f, .207843f, .241176f, .274510f, .305882f, .337255f,
                                                .370588f, .403922f, .435294f, .466667f, .5f,      .533333f, .564706f, .596078f, .629412f, .662745f, .694118f,
                                                .725490f, .758824f, .792157f, .823529f, .854902f, .888235f, .921569f, .952941f, .984314f, 1e+37f};
    static constexpr float Midpoints6bit[64] = {.007843f, .023529f, .039216f, .054902f, .070588f, .086275f, .101961f, .117647f, .133333f, .149020f, .164706f,
                                                .180392f, .196078f, .211765f, .227451f, .245098f, .262745f, .278431f, .294118f, .309804f, .325490f, .341176f,
                                                .356863f, .372549f, .388235f, .403922f, .419608f, .435294f, .450980f, .466667f, .482353f, .500000f, .517647f,
                                                .533333f, .549020f, .564706f, .580392f, .596078f, .611765f, .627451f, .643137f, .658824f, .674510f, .690196f,
                                                .705882f, .721569f, .737255f, .754902f, .772549f, .788235f, .803922f, .819608f, .835294f, .850980f, .866667f,
                                                .882353f, .898039f, .913725f, .929412f, .945098f, .960784f, .976471f, .992157f, 1e+37f};
};
#pragma pack(pop)
}  // namespace quicktex