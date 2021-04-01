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

class alignas(8) BC4Block {
   public:
    static constexpr int Width = 4;
    static constexpr int Height = 4;

    using SelectorArray = std::array<std::array<uint8_t, Width>, Height>;
    using AlphaPair = std::pair<uint8_t, uint8_t>;

    constexpr BC4Block() {
        static_assert(sizeof(BC4Block) == 8);
        static_assert(sizeof(std::array<BC4Block, 10>) == 8 * 10);
        alpha0 = alpha1 = 0;
        SetSelectorBits(0);
    }

    constexpr BC4Block(uint8_t valpha0, uint8_t valpha1, const SelectorArray& selectors) {
        alpha0 = valpha0;
        alpha1 = valpha1;
        SetSelectors(selectors);
    }

    inline bool Is6Value() const { return alpha0 <= alpha1; }

    AlphaPair GetAlphas() const { return AlphaPair(alpha0, alpha1); }

    void SetAlphas(AlphaPair as) {
        alpha0 = as.first;
        alpha1 = as.second;
    }

    inline uint64_t GetSelectorBits() const {
        auto packed = Pack<uint8_t, uint64_t, 8, SelectorSize>(selectors);
        assert(packed <= SelectorBitsMax);
        return packed;
    }

    void SetSelectorBits(uint64_t packed) {
        assert(packed <= SelectorBitsMax);
        selectors = Unpack<uint64_t, uint8_t, 8, SelectorSize>(packed);
    }

    SelectorArray GetSelectors() const {
        SelectorArray unpacked;
        auto rows = Unpack<uint64_t, uint16_t, 12, Width>(GetSelectorBits());
        for (unsigned i = 0; i < Height; i++) {
            auto row = Unpack<uint16_t, uint8_t, SelectorBits, Width>(rows[i]);
            unpacked[i] = row;
        }

        return unpacked;
    }

    void SetSelectors(const SelectorArray& unpacked) {
        std::array<uint16_t, Height> rows;
        for (int i = 0; i < Height; i++) { rows[i] = Pack<uint8_t, uint16_t, SelectorBits, Width>(unpacked[i]); }
        auto packed = Pack<uint16_t, uint64_t, 12, Height>(rows);
        SetSelectorBits(packed);
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

    uint8_t alpha0;
    uint8_t alpha1;

   private:
    std::array<uint8_t, SelectorSize> selectors;
};
}  // namespace quicktex::s3tc
