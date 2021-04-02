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
    using ColorPair = std::pair<Color, Color>;

    constexpr BC1Block() {
        static_assert(sizeof(BC1Block) == 8);
        static_assert(sizeof(std::array<BC1Block, 10>) == 8 * 10);
        static_assert(alignof(BC1Block) >= 8);
        _color0 = _color1 = {0, 0};
        _selectors = {0, 0, 0, 0};
    }

    BC1Block(Color color0, Color color1, const SelectorArray& selectors) {
        SetColor0(color0);
        SetColor1(color1);
        SetSelectors(selectors);
    }

    BC1Block(uint16_t ep0, uint16_t ep1, const SelectorArray& selectors) {
        SetColor0Raw(ep0);
        SetColor1Raw(ep1);
        SetSelectors(selectors);
    }

    BC1Block(uint16_t ep0, uint16_t ep1, uint8_t solid_mask) {
        SetColor0Raw(ep0);
        SetColor1Raw(ep1);
        _selectors.fill(solid_mask);
    }

    constexpr uint16_t GetColor0Raw() const { return Pack<uint8_t, uint16_t, 8, EndpointSize>(_color0); }
    constexpr uint16_t GetColor1Raw() const { return Pack<uint8_t, uint16_t, 8, EndpointSize>(_color1); }

    void SetColor0Raw(uint16_t c) { _color0 = Unpack<uint16_t, uint8_t, 8, EndpointSize>(c); }
    void SetColor1Raw(uint16_t c) { _color1 = Unpack<uint16_t, uint8_t, 8, EndpointSize>(c); }

    Color GetColor0() const { return Color::Unpack565(GetColor0Raw()); }
    Color GetColor1() const { return Color::Unpack565(GetColor1Raw()); }
    ColorPair GetColors() const { return {GetColor0(), GetColor1()}; }

    void SetColor0(Color c) { SetColor0Raw(c.Pack565()); }
    void SetColor1(Color c) { SetColor1Raw(c.Pack565()); }
    void SetColors(ColorPair cs) {
        SetColor0(cs.first);
        SetColor1(cs.second);
    }

    constexpr SelectorArray GetSelectors() const { return MapArray(_selectors, Unpack<uint8_t, uint8_t, SelectorBits, Width>); }

    void SetSelectors(const SelectorArray& unpacked) { _selectors = MapArray(unpacked, Pack<uint8_t, uint8_t, SelectorBits, Width>); }

    constexpr bool Is3Color() const { return GetColor0Raw() <= GetColor1Raw(); }

    constexpr static inline size_t EndpointSize = 2;   // in bytes
    constexpr static inline size_t SelectorSize = 4;   // in bytes
    constexpr static inline uint8_t SelectorBits = 2;  // in bits

   private:
    std::array<uint8_t, EndpointSize> _color0;
    std::array<uint8_t, EndpointSize> _color1;
    std::array<uint8_t, SelectorSize> _selectors;
};
}  // namespace quicktex::s3tc