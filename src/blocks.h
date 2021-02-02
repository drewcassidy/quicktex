/*  Python-rgbcx Texture Compression Library
    Copyright (C) 2021 Andrew Cassidy <drewcassidy@me.com>
    Partially derived from rgbcx.h written by Richard Geldreich 2020 <richgel99@gmail.com>
    and licenced under the public domain

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "util.h"
#include <cassert>
#include <cstdint>
#include <cstdlib>

constexpr inline uint8_t DXT1SelectorBits = 2U;

#pragma pack(push, 1)
struct Color32 {
    union {
        struct {
            uint8_t R;
            uint8_t G;
            uint8_t B;
            uint8_t A;
        };

        uint8_t C[4];
    };

    Color32() {}

    Color32(uint32_t vr, uint32_t vg, uint32_t vb, uint32_t va);

    void set(uint8_t vr, uint8_t vg, uint8_t vb, uint8_t va);

    void set(const Color32 &other);

    uint8_t operator[](uint32_t idx) const;
    uint8_t &operator[](uint32_t idx);

    bool operator==(const Color32 &rhs) const {
        return R == rhs.R && G == rhs.G && B == rhs.B && A == rhs.A;
    }

    static Color32 min(const Color32 &a, const Color32 &b);
    static Color32 max(const Color32 &a, const Color32 &b);
};

struct BC1Block {
    constexpr static inline size_t EndpointSize = 2;
    constexpr static inline size_t SelectorSize = 4;
    constexpr static inline uint8_t SelectorBits = 2;
    constexpr static inline uint8_t SelectorValues = 1 << SelectorBits;
    constexpr static inline uint8_t SelectorMask = SelectorValues - 1;

    uint8_t LowColor[EndpointSize];
    uint8_t HighColor[EndpointSize];
    uint8_t Selectors[SelectorSize];

    inline uint32_t get_low_color() const { return LowColor[0] | (LowColor[1] << 8U); }
    inline uint32_t get_high_color() const { return HighColor[0] | (HighColor[1] << 8U); }
    inline bool is_3color() const { return get_low_color() <= get_high_color(); }
    inline void set_low_color(uint16_t c) {
        LowColor[0] = static_cast<uint8_t>(c & 0xFF);
        LowColor[1] = static_cast<uint8_t>((c >> 8) & 0xFF);
    }
    inline void set_high_color(uint16_t c) {
        HighColor[0] = static_cast<uint8_t>(c & 0xFF);
        HighColor[1] = static_cast<uint8_t>((c >> 8) & 0xFF);
    }
    inline uint32_t get_selector(uint32_t x, uint32_t y) const {
        assert((x < 4U) && (y < 4U));
        return (Selectors[y] >> (x * SelectorBits)) & SelectorMask;
    }
    inline void set_selector(uint32_t x, uint32_t y, uint32_t val) {
        assert((x < 4U) && (y < 4U) && (val < 4U));
        Selectors[y] &= (~(SelectorMask << (x * SelectorBits)));
        Selectors[y] |= (val << (x * DXT1SelectorBits));
    }

    static inline uint16_t pack_color(const Color32 &color, bool scaled, uint32_t bias = 127U) {
        uint32_t r = color.R, g = color.G, b = color.B;
        if (scaled) {
            r = (r * 31U + bias) / 255U;
            g = (g * 63U + bias) / 255U;
            b = (b * 31U + bias) / 255U;
        }
        return static_cast<uint16_t>(minimum(b, 31U) | (minimum(g, 63U) << 5U) | (minimum(r, 31U) << 11U));
    }

    static inline uint16_t pack_unscaled_color(uint32_t r, uint32_t g, uint32_t b) { return static_cast<uint16_t>(b | (g << 5U) | (r << 11U)); }

    static inline void unpack_color(uint32_t c, uint32_t &r, uint32_t &g, uint32_t &b) {
        r = (c >> 11) & 31;
        g = (c >> 5) & 63;
        b = c & 31;

        r = (r << 3) | (r >> 2);
        g = (g << 2) | (g >> 4);
        b = (b << 3) | (b >> 2);
    }

    static inline void unpack_color_unscaled(uint32_t c, uint32_t &r, uint32_t &g, uint32_t &b) {
        r = (c >> 11) & 31;
        g = (c >> 5) & 63;
        b = c & 31;
    }
};

struct BC4Block {
    constexpr static inline size_t EndpointSize = 1;
    constexpr static inline size_t SelectorSize = 6;
    constexpr static inline uint8_t SelectorBits = 3;
    constexpr static inline uint8_t SelectorValues = 1 << SelectorBits;
    constexpr static inline uint8_t SelectorMask = SelectorValues - 1;

    uint8_t LowAlpha;
    uint8_t HighAlpha;
    uint8_t Selectors[SelectorSize];

    inline uint32_t get_low_alpha() const { return LowAlpha; }
    inline uint32_t get_high_alpha() const { return HighAlpha; }
    inline bool is_alpha6_block() const { return get_low_alpha() <= get_high_alpha(); }

    inline uint64_t get_selector_bits() const {
        return ((uint64_t)((uint32_t)Selectors[0] | ((uint32_t)Selectors[1] << 8U) | ((uint32_t)Selectors[2] << 16U) |
                           ((uint32_t)Selectors[3] << 24U))) |
               (((uint64_t)Selectors[4]) << 32U) | (((uint64_t)Selectors[5]) << 40U);
    }

    inline uint32_t get_selector(uint32_t x, uint32_t y, uint64_t selector_bits) const {
        assert((x < 4U) && (y < 4U));
        return (selector_bits >> (((y * 4) + x) * SelectorBits)) & (SelectorMask);
    }

    static inline uint32_t get_block_values6(uint8_t *pDst, uint32_t l, uint32_t h) {
        pDst[0] = static_cast<uint8_t>(l);
        pDst[1] = static_cast<uint8_t>(h);
        pDst[2] = static_cast<uint8_t>((l * 4 + h) / 5);
        pDst[3] = static_cast<uint8_t>((l * 3 + h * 2) / 5);
        pDst[4] = static_cast<uint8_t>((l * 2 + h * 3) / 5);
        pDst[5] = static_cast<uint8_t>((l + h * 4) / 5);
        pDst[6] = 0;
        pDst[7] = 255;
        return 6;
    }

    static inline uint32_t get_block_values8(uint8_t *pDst, uint32_t l, uint32_t h) {
        pDst[0] = static_cast<uint8_t>(l);
        pDst[1] = static_cast<uint8_t>(h);
        pDst[2] = static_cast<uint8_t>((l * 6 + h) / 7);
        pDst[3] = static_cast<uint8_t>((l * 5 + h * 2) / 7);
        pDst[4] = static_cast<uint8_t>((l * 4 + h * 3) / 7);
        pDst[5] = static_cast<uint8_t>((l * 3 + h * 4) / 7);
        pDst[6] = static_cast<uint8_t>((l * 2 + h * 5) / 7);
        pDst[7] = static_cast<uint8_t>((l + h * 6) / 7);
        return 8;
    }

    static inline uint32_t get_block_values(uint8_t *pDst, uint32_t l, uint32_t h) {
        if (l > h)
            return get_block_values8(pDst, l, h);
        else
            return get_block_values6(pDst, l, h);
    }
};

struct BC3Block {
    BC4Block AlphaBlock;
    BC1Block ColorBlock;
};

struct BC5Block {
    BC4Block RBlock;
    BC4Block GBlock;
};

#pragma pack(pop)