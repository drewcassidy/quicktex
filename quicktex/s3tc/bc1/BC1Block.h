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

#include <array>
#include <cstdint>
#include <cstdlib>
#include <utility>

#include "../../Color.h"

namespace quicktex::s3tc {

class alignas(8) BC1Block {
   public:
    static constexpr size_t Width = 4;
    static constexpr size_t Height = 4;

    static constexpr size_t EndpointSize = 2;                        // size of a 5:6:5 endpoint in bytes
    static constexpr size_t SelectorSize = 4;                        // size of selector array in bytes
    static constexpr size_t SelectorBits = 2;                        // size of a selector in bits
    static constexpr uint8_t SelectorMax = (1 << SelectorBits) - 1;  // maximum value of a selector

    using SelectorArray = std::array<std::array<uint8_t, Width>, Height>;
    using ColorPair = std::pair<Color, Color>;

   private:
    std::array<uint8_t, EndpointSize> _color0;
    std::array<uint8_t, EndpointSize> _color1;
    std::array<uint8_t, SelectorSize> _selectors;

   public:
    /// Create a new BC1Blok
    constexpr BC1Block() : _color0(), _color1(), _selectors() {
        static_assert(sizeof(BC1Block) == 8);
        static_assert(sizeof(std::array<BC1Block, 10>) == 8 * 10);
        static_assert(alignof(BC1Block) >= 8);
    }

    /**
     * Create a new BC1Block
     * @param color0 first endpoint color
     * @param color1 second endpoint color
     * @param selectors the selectors as a 4x4 list of integers, between 0 and 3 inclusive.
     */
    BC1Block(Color color0, Color color1, const SelectorArray& selectors) {
        SetColor0(color0);
        SetColor1(color1);
        SetSelectors(selectors);
    }

    /**
     * Create a new BC1Block
     * @param ep0 first endpoint
     * @param ep1 second endpoint
     * @param selectors the selectors as a 4x4 list of integers, between 0 and 3 inclusive.
     */
    BC1Block(uint16_t ep0, uint16_t ep1, const SelectorArray& selectors) {
        SetColor0Raw(ep0);
        SetColor1Raw(ep1);
        SetSelectors(selectors);
    }

    /**
     * Create a new BC1Block
     * @param ep0 first endpoint
     * @param ep1 second endpoint
     * @param solid_mask single byte mask to use for each row
     */
    BC1Block(uint16_t ep0, uint16_t ep1, uint8_t solid_mask) {
        SetColor0Raw(ep0);
        SetColor1Raw(ep1);
        _selectors.fill(solid_mask);
    }

    uint16_t GetColor0Raw() const;
    uint16_t GetColor1Raw() const;

    void SetColor0Raw(uint16_t c);
    void SetColor1Raw(uint16_t c);

    Color GetColor0() const { return Color::Unpack565(GetColor0Raw()); }
    Color GetColor1() const { return Color::Unpack565(GetColor1Raw()); }
    ColorPair GetColors() const { return {GetColor0(), GetColor1()}; }

    void SetColor0(Color c) { SetColor0Raw(c.Pack565()); }
    void SetColor1(Color c) { SetColor1Raw(c.Pack565()); }
    void SetColors(ColorPair cs) {
        SetColor0(cs.first);
        SetColor1(cs.second);
    }

    /**
     * Get this block's selectors
     * @return a 4x4 array of integers between 0 and 3 inclusive
     */
    SelectorArray GetSelectors() const;

    /**
     * Set this block's selectors
     * @param unpacked a 4x4 array of integers between 0 and 3 inclusive
     */
    void SetSelectors(const SelectorArray& unpacked);

    bool Is3Color() const { return GetColor0Raw() <= GetColor1Raw(); }

    bool operator==(const BC1Block& Rhs) const;
    bool operator!=(const BC1Block& Rhs) const;
};
}  // namespace quicktex::s3tc