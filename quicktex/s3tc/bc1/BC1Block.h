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

#include "../../Block.h"
#include "../../Color.h"
#include "../../util.h"

namespace quicktex::s3tc {

#pragma pack(push, 1)
class BC1Block : public Block<4, 4> {
   public:
    using UnpackedSelectors = std::array<std::array<uint8_t, Width>, Height>;

    constexpr BC1Block() { static_assert(sizeof(BC1Block) == 8); }

    uint16_t GetLowColor() const { return static_cast<uint16_t>(_low_color[0] | (_low_color[1] << 8U)); }
    uint16_t GetHighColor() const { return static_cast<uint16_t>(_high_color[0] | (_high_color[1] << 8U)); }
    Color GetLowColor32() const { return Color::Unpack565(GetLowColor()); }
    Color GetHighColor32() const { return Color::Unpack565(GetHighColor()); }

    bool Is3Color() const { return GetLowColor() <= GetHighColor(); }
    void SetLowColor(uint16_t c) {
        _low_color[0] = c & 0xFF;
        _low_color[1] = (c >> 8) & 0xFF;
    }
    void SetHighColor(uint16_t c) {
        _high_color[0] = c & 0xFF;
        _high_color[1] = (c >> 8) & 0xFF;
    }
    uint32_t GetSelector(uint32_t x, uint32_t y) const {
        assert((x < 4U) && (y < 4U));
        return (selectors[y] >> (x * SelectorBits)) & SelectorMask;
    }
    void SetSelector(uint32_t x, uint32_t y, uint32_t val) {
        assert((x < 4U) && (y < 4U) && (val < 4U));
        selectors[y] &= (~(SelectorMask << (x * SelectorBits)));
        selectors[y] |= (val << (x * SelectorBits));
    }

    UnpackedSelectors UnpackSelectors() const {
        UnpackedSelectors unpacked;
        for (unsigned i = 0; i < 4; i++) { unpacked[i] = Unpack<uint8_t, uint8_t, 2, 4>(selectors[i]); }
        return unpacked;
    }

    void PackSelectors(const UnpackedSelectors& unpacked, uint8_t mask = 0) {
        for (unsigned i = 0; i < 4; i++) { selectors[i] = mask ^ Pack<uint8_t, uint8_t, 2, 4>(unpacked[i]); }
    }

    constexpr static inline size_t EndpointSize = 2;
    constexpr static inline size_t SelectorSize = 4;
    constexpr static inline uint8_t SelectorBits = 2;
    constexpr static inline uint8_t SelectorValues = 1 << SelectorBits;
    constexpr static inline uint8_t SelectorMask = SelectorValues - 1;

   private:
    std::array<uint8_t, EndpointSize> _low_color;
    std::array<uint8_t, EndpointSize> _high_color;

   public:
    std::array<uint8_t, 4> selectors;
};
#pragma pack(pop)
}  // namespace quicktex::s3tc