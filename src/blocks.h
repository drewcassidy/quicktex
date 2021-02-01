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
#include <cstdlib>
#include <cstdint>

constexpr inline uint8_t DXT1SelectorBits = 2U;

struct color32 {
    union {
        struct {
            uint8_t r;
            uint8_t g;
            uint8_t b;
            uint8_t a;
        };

        uint8_t c[4];

        uint32_t m;
    };

    color32() {}

    color32(uint32_t vr, uint32_t vg, uint32_t vb, uint32_t va);

    void set(uint8_t vr, uint8_t vg, uint8_t vb, uint8_t va);

    void set_rgb(const color32 &other);

    uint8_t operator[](uint32_t idx) const;
    uint8_t &operator[](uint32_t idx);

    bool operator==(const color32 &rhs) const { return m == rhs.m; }

    static color32 comp_min(const color32 &a, const color32 &b);
    static color32 comp_max(const color32 &a, const color32 &b);
};

struct bc1_block {
    constexpr static inline size_t EndpointSize = 2;
    constexpr static inline size_t SelectorSize = 4;
    constexpr static inline uint8_t SelectorBits = 2;
    constexpr static inline uint8_t SelectorValues = 1 << SelectorBits;
    constexpr static inline uint8_t SelectorMask = SelectorValues - 1;

    uint8_t m_low_color[EndpointSize];
    uint8_t m_high_color[EndpointSize];
    uint8_t m_selectors[SelectorSize];

    inline uint32_t get_low_color() const { return m_low_color[0] | (m_low_color[1] << 8U); }
    inline uint32_t get_high_color() const { return m_high_color[0] | (m_high_color[1] << 8U); }
    inline bool is_3color() const { return get_low_color() <= get_high_color(); }
    inline void set_low_color(uint16_t c) {
        m_low_color[0] = static_cast<uint8_t>(c & 0xFF);
        m_low_color[1] = static_cast<uint8_t>((c >> 8) & 0xFF);
    }
    inline void set_high_color(uint16_t c) {
        m_high_color[0] = static_cast<uint8_t>(c & 0xFF);
        m_high_color[1] = static_cast<uint8_t>((c >> 8) & 0xFF);
    }
    inline uint32_t get_selector(uint32_t x, uint32_t y) const {
        assert((x < 4U) && (y < 4U));
        return (m_selectors[y] >> (x * SelectorBits)) & SelectorMask;
    }
    inline void set_selector(uint32_t x, uint32_t y, uint32_t val) {
        assert((x < 4U) && (y < 4U) && (val < 4U));
        m_selectors[y] &= (~(SelectorMask << (x * SelectorBits)));
        m_selectors[y] |= (val << (x * DXT1SelectorBits));
    }

    static inline uint16_t pack_color(const color32 &color, bool scaled, uint32_t bias = 127U) {
        uint32_t r = color.r, g = color.g, b = color.b;
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

struct bc4_block {
    constexpr static inline size_t EndpointSize = 1;
    constexpr static inline size_t SelectorSize = 6;
    constexpr static inline uint8_t SelectorBits = 3;
    constexpr static inline uint8_t SelectorValues = 1 << SelectorBits;
    constexpr static inline uint8_t SelectorMask = SelectorValues - 1;

    uint8_t m_endpoints[EndpointSize * 2];
    uint8_t m_selectors[SelectorSize];

    inline uint32_t get_low_alpha() const { return m_endpoints[0]; }
    inline uint32_t get_high_alpha() const { return m_endpoints[1]; }
    inline bool is_alpha6_block() const { return get_low_alpha() <= get_high_alpha(); }

    inline uint64_t get_selector_bits() const {
        return ((uint64_t)((uint32_t)m_selectors[0] | ((uint32_t)m_selectors[1] << 8U) | ((uint32_t)m_selectors[2] << 16U) |
                           ((uint32_t)m_selectors[3] << 24U))) |
               (((uint64_t)m_selectors[4]) << 32U) | (((uint64_t)m_selectors[5]) << 40U);
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

struct bc3_block {
    bc4_block alpha_block;
    bc1_block color_block;
};

struct bc5_block {
    bc4_block r_block;
    bc4_block g_block;
};
