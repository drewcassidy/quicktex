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
#include "../bc1/BC1Block.h"

namespace quicktex::s3tc {

#pragma pack(push, 1)
class BC4Block : public Block<4, 4> {
   public:
    using UnpackedSelectors = std::array<std::array<uint8_t, 4>, 4>;

    inline uint32_t GetLowAlpha() const { return low_alpha; }
    inline uint32_t GetHighAlpha() const { return high_alpha; }
    inline bool Is6Alpha() const { return GetLowAlpha() <= GetHighAlpha(); }

    inline uint64_t GetSelectorBits() const {
        auto packed = Pack<uint8_t, uint64_t, 8, 6>(selectors);
        assert(packed <= SelectorBitsMax);
        return packed;
    }

    void SetSelectorBits(uint64_t packed) {
        assert(packed <= SelectorBitsMax);
        selectors = Unpack<uint64_t, uint8_t, 8, 6>(packed);
    }

    UnpackedSelectors UnpackSelectors() const {
        UnpackedSelectors unpacked;
        auto rows = Unpack<uint64_t, uint16_t, 12, 4>(GetSelectorBits());
        for (unsigned i = 0; i < 4; i++) {
            auto row = Unpack<uint16_t, uint8_t, SelectorBits, 4>(rows[i]);
            unpacked[i] = row;
        }

        return unpacked;
    }

    void PackSelectors(const UnpackedSelectors& unpacked) {
        std::array<uint16_t, 4> rows;
        for (unsigned i = 0; i < 4; i++) { rows[i] = Pack<uint8_t, uint16_t, SelectorBits, 4>(unpacked[i]); }
        auto packed = Pack<uint16_t, uint64_t, 12, 4>(rows);
        SetSelectorBits(packed);
    }

    void PackSelectors(const std::array<uint8_t, 16>& unpacked) {
        auto packed = Pack<uint8_t, uint64_t, 3, 16>(unpacked);
        SetSelectorBits(packed);
    }

    inline uint32_t GetSelector(uint32_t x, uint32_t y, uint64_t selector_bits) const {
        assert((x < 4U) && (y < 4U));
        return (selector_bits >> (((y * 4) + x) * SelectorBits)) & (SelectorMask);
    }

    static inline std::array<uint8_t, 8> GetValues6(uint32_t l, uint32_t h) {
        return {static_cast<uint8_t>(l),
                static_cast<uint8_t>(h),
                static_cast<uint8_t>((l * 4 + h) / 5),
                static_cast<uint8_t>((l * 3 + h * 2) / 5),
                static_cast<uint8_t>((l * 2 + h * 3) / 5),
                static_cast<uint8_t>((l + h * 4) / 5),
                0,
                255};
    }

    static inline std::array<uint8_t, 8> GetValues8(uint32_t l, uint32_t h) {
        return {static_cast<uint8_t>(l),
                static_cast<uint8_t>(h),
                static_cast<uint8_t>((l * 6 + h) / 7),
                static_cast<uint8_t>((l * 5 + h * 2) / 7),
                static_cast<uint8_t>((l * 4 + h * 3) / 7),
                static_cast<uint8_t>((l * 3 + h * 4) / 7),
                static_cast<uint8_t>((l * 2 + h * 5) / 7),
                static_cast<uint8_t>((l + h * 6) / 7)};
    }

    static inline std::array<uint8_t, 8> GetValues(uint32_t l, uint32_t h) {
        if (l > h)
            return GetValues8(l, h);
        else
            return GetValues6(l, h);
    }

    constexpr static inline size_t EndpointSize = 1;
    constexpr static inline size_t SelectorSize = 6;
    constexpr static inline uint8_t SelectorBits = 3;
    constexpr static inline uint8_t SelectorValues = 1 << SelectorBits;
    constexpr static inline uint8_t SelectorMask = SelectorValues - 1;
    constexpr static inline uint64_t SelectorBitsMax = (1ULL << (8U * SelectorSize)) - 1U;

    uint8_t low_alpha;
    uint8_t high_alpha;
    std::array<uint8_t, SelectorSize> selectors;
};
#pragma pack(pop)
}  // namespace quicktex::s3tc
