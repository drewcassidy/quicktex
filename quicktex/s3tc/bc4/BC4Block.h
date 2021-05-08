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

namespace quicktex::s3tc {

class alignas(8) BC4Block {
   public:
    static constexpr size_t Width = 4;
    static constexpr size_t Height = 4;

    static constexpr size_t SelectorSize = 6;                       // size of selector array in bytes
    static constexpr size_t SelectorBits = 3;                       // size of a selector in bits
    static constexpr size_t SelectorMax = (1 << SelectorBits) - 1;  // maximum value of a selector

    using SelectorArray = std::array<std::array<uint8_t, Width>, Height>;
    using AlphaPair = std::pair<uint8_t, uint8_t>;

    uint8_t alpha0;  // first endpoint
    uint8_t alpha1;  // second endpoint

   private:
    std::array<uint8_t, SelectorSize> _selectors;  // internal array of selector bytes

   public:
    // Constructors

    /// Create a new BC4Block
    constexpr BC4Block() : alpha0(0), alpha1(0), _selectors() {
        static_assert(sizeof(BC4Block) == 8);
        static_assert(sizeof(std::array<BC4Block, 10>) == 8 * 10);
        static_assert(alignof(BC4Block) >= 8);
    }

    /**
     * Create a new BC4Block
     * @param valpha0 first endpoint value
     * @param valpha1 second endpoint value
     * @param selectors the selectors as a 4x4 array of integers, between 0 and 7 inclusive.
     */
    BC4Block(uint8_t valpha0, uint8_t valpha1, const SelectorArray& selectors) {
        alpha0 = valpha0;
        alpha1 = valpha1;
        SetSelectors(selectors);
    }

    /**
     * Create a new solid BC4Block
     * @param alpha first endpoint value
     */
    BC4Block(uint8_t alpha) {
        alpha0 = alpha;
        alpha1 = alpha;
        _selectors.fill(0);
    }

    /// Get a alpha0 and alpha1 as a pair
    AlphaPair GetAlphas() const { return AlphaPair(alpha0, alpha1); }

    /// Set alpha0 and alpha1 as a pair
    void SetAlphas(AlphaPair as) {
        alpha0 = as.first;
        alpha1 = as.second;
    }

    /// Get the block's selectors as a 4x4 array of integers between 0 and 7 inclusive.
    SelectorArray GetSelectors() const;

    /// Get the block's selectors as a 4x4 array of integers between 0 and 7 inclusive.
    void SetSelectors(const SelectorArray& unpacked);

    /// True if the block uses 6-value interpolation, i.e. alpha0 <= alpha1.
    bool Is6Value() const { return alpha0 <= alpha1; }

    /// The interpolated values of this block as an array of 8 integers.
    std::array<uint8_t, 8> GetValues() const { return Is6Value() ? GetValues6() : GetValues8(); }

    bool operator==(const BC4Block& Rhs) const;
    bool operator!=(const BC4Block& Rhs) const;

   private:
    std::array<uint8_t, 8> GetValues6() const;
    std::array<uint8_t, 8> GetValues8() const;
};
}  // namespace quicktex::s3tc
