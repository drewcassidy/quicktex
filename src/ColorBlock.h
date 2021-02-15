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
#include <cstdio>
#include <vector>

#include "Color.h"

template <size_t N> class ColorRow {
   public:
    constexpr Color &operator[](size_t n) noexcept { return _colors[n]; }
    constexpr const Color &operator[](size_t n) const noexcept { return _colors[n]; }

    constexpr int size() noexcept { return N; }

   private:
    std::array<Color, 4> _colors;
};

template <size_t M, size_t N> class ColorBlock {
   public:
    using Row = ColorRow<N>;
    constexpr Row &operator[](size_t n) noexcept { return *_rows[n]; }
    constexpr const Row &operator[](size_t n) const noexcept { return *_rows[n]; }

    constexpr int width() noexcept { return N; }
    constexpr int height() noexcept { return M; }
    constexpr int size() noexcept { return N * M; }

    ColorBlock(const std::array<Row *, M> &Rows) : _rows(Rows) {}

   private:
    std::array<Row *, M> _rows;
};

using Color4x4 = ColorBlock<4, 4>;
