/*  Python-rgbcx Texture Compression Library
    Copyright (C) 2021 Andrew Cassidy <drewcassidy@me.com>
    Partially derived from rgbcx.h written by Richard Geldreich 2020 <richgel99@gmail.com>
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

#include "color.h"
#include "util.h"

#pragma pack(push, 1)
class BC1Block {
   public:
    using UnpackedSelectors = std::array<std::array<uint8_t, 4>, 4>;

    uint16_t GetLowColor() const { return _low_color[0] | _low_color[1] << 8U; }
    uint16_t GetHighColor() const { return _high_color[0] | _high_color[1] << 8U; }
    Color32 GetLowColor32() const { return Color32::Unpack565(GetLowColor()); }
    Color32 GetHighColor32() const { return Color32::Unpack565(GetHighColor()); }

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
        for (int i = 0; i < 4; i++) {
            unpacked[i] = Unpack<uint8_t, uint8_t, 2, 4>(selectors[i]);
        }
        return unpacked;
    }

    void PackSelectors(const UnpackedSelectors& unpacked) {
        for (int i = 0; i < 4; i++) {
            selectors[i] = Pack<uint8_t, uint8_t, 2, 4>(unpacked[i]);
        }
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

class BC4Block {
   public:
    inline uint32_t GetLowAlpha() const { return low_alpha; }
    inline uint32_t GetHighAlpha() const { return high_alpha; }
    inline bool Is6Alpha() const { return GetLowAlpha() <= GetHighAlpha(); }

    inline uint64_t GetSelectorBits() const {
        return ((uint64_t)((uint32_t)selectors[0] | ((uint32_t)selectors[1] << 8U) | ((uint32_t)selectors[2] << 16U) | ((uint32_t)selectors[3] << 24U))) |
               (((uint64_t)selectors[4]) << 32U) | (((uint64_t)selectors[5]) << 40U);
    }

    inline uint32_t GetSelector(uint32_t x, uint32_t y, uint64_t selector_bits) const {
        assert((x < 4U) && (y < 4U));
        return (selector_bits >> (((y * 4) + x) * SelectorBits)) & (SelectorMask);
    }

    static inline std::array<uint8_t, 8> GetBlockValues6(uint32_t l, uint32_t h) {
        return {static_cast<uint8_t>(l),
                static_cast<uint8_t>(h),
                static_cast<uint8_t>((l * 4 + h) / 5),
                static_cast<uint8_t>((l * 3 + h * 2) / 5),
                static_cast<uint8_t>((l * 2 + h * 3) / 5),
                static_cast<uint8_t>((l + h * 4) / 5),
                0,
                255};
    }

    static inline std::array<uint8_t, 8> GetBlockValues8(uint32_t l, uint32_t h) {
        return {static_cast<uint8_t>(l),
                static_cast<uint8_t>(h),
                static_cast<uint8_t>((l * 6 + h) / 7),
                static_cast<uint8_t>((l * 5 + h * 2) / 7),
                static_cast<uint8_t>((l * 4 + h * 3) / 7),
                static_cast<uint8_t>((l * 3 + h * 4) / 7),
                static_cast<uint8_t>((l * 2 + h * 5) / 7),
                static_cast<uint8_t>((l + h * 6) / 7)};
    }

    static inline std::array<uint8_t, 8> GetBlockValues(uint32_t l, uint32_t h) {
        if (l > h)
            return GetBlockValues8(l, h);
        else
            return GetBlockValues6(l, h);
    }

    constexpr static inline size_t EndpointSize = 1;
    constexpr static inline size_t SelectorSize = 6;
    constexpr static inline uint8_t SelectorBits = 3;
    constexpr static inline uint8_t SelectorValues = 1 << SelectorBits;
    constexpr static inline uint8_t SelectorMask = SelectorValues - 1;

    uint8_t low_alpha;
    uint8_t high_alpha;
    uint8_t selectors[SelectorSize];
};

class BC3Block {
   public:
    BC4Block alpha_block;
    BC1Block color_block;
};

class BC5Block {
   public:
    BC4Block r_block;
    BC4Block g_block;
};
#pragma pack(pop)