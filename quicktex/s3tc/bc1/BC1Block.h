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
#include <cassert>
#include <cstdint>
#include <cstdlib>

#include "../../Color.h"
#include "../../util.h"

namespace quicktex::s3tc {

class alignas(8) BC1Block {
   public:
    static constexpr int Width = 4;
    static constexpr int Height = 4;

    using SelectorArray = std::array<std::array<uint8_t, Width>, Height>;
    using ColorPair = std::tuple<Color, Color>;

    constexpr BC1Block() {
        static_assert(sizeof(BC1Block) == 8);
        static_assert(sizeof(std::array<BC1Block, 10>) == 8 * 10);
        SetColor0Raw(0);
        SetColor1Raw(0);
        SetSelectorsSolid(0);
    }

    constexpr BC1Block(Color color0, Color color1, const SelectorArray& selectors) {
        SetColor0(color0);
        SetColor1(color1);
        SetSelectors(selectors);
    }

    constexpr BC1Block(Color color0, Color color1, uint8_t solid_mask) {
        SetColor0(color0);
        SetColor1(color1);
        SetSelectorsSolid(solid_mask);
    }

    uint16_t GetColor0Raw() const { return static_cast<uint16_t>(_color_0[0] | (_color_0[1] << 8U)); }
    uint16_t GetColor1Raw() const { return static_cast<uint16_t>(_color_1[0] | (_color_1[1] << 8U)); }
    void SetColor0Raw(uint16_t c) {
        _color_0[0] = c & 0xFF;
        _color_0[1] = (c >> 8) & 0xFF;
    }
    void SetColor1Raw(uint16_t c) {
        _color_1[0] = c & 0xFF;
        _color_1[1] = (c >> 8) & 0xFF;
    }

    Color GetColor0() const { return Color::Unpack565(GetColor0Raw()); }
    Color GetColor1() const { return Color::Unpack565(GetColor1Raw()); }
    ColorPair GetColors() const { return {GetColor0(), GetColor1()}; }

    void SetColor0(Color c) { SetColor0Raw(c.Pack565()); }
    void SetColor1(Color c) { SetColor1Raw(c.Pack565()); }
    void SetColors(ColorPair cs) {
        SetColor0(std::get<0>(cs));
        SetColor1(std::get<1>(cs));
    }

    bool Is3Color() const { return GetColor0Raw() <= GetColor1Raw(); }

    SelectorArray GetSelectors() const {
        SelectorArray unpacked;
        for (int i = 0; i < Height; i++) { unpacked[i] = Unpack<uint8_t, uint8_t, SelectorBits, Width>(_selectors[i]); }
        return unpacked;
    }

    void SetSelectors(const SelectorArray& unpacked) {
        for (int i = 0; i < Height; i++) { _selectors[i] = Pack<uint8_t, uint8_t, SelectorBits, Width>(unpacked[i]); }
    }

    /**
     * Set every row of selectors to the same 8-bit mask. useful for solid-color blocks
     * @param mask the 8-bit mask to use for each row
     */
    void SetSelectorsSolid(uint8_t mask) {
        for (int i = 0; i < Height; i++) _selectors[i] = mask;
    }

    constexpr static inline size_t EndpointSize = 2; // in bytes
    constexpr static inline size_t SelectorSize = 4; // in bytes
    constexpr static inline uint8_t SelectorBits = 2; // in bits

   private:
    std::array<uint8_t, EndpointSize> _color_0;
    std::array<uint8_t, EndpointSize> _color_1;
    std::array<uint8_t, SelectorSize> _selectors;
};
}  // namespace quicktex::s3tc