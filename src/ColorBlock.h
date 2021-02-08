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
#include <cstdint>
#include <span>

#include "blocks.h"

template <size_t M, size_t N, class T> class ColorBlock {
   public:
    using row = std::span<T, N>;

    ColorBlock(const std::array<T *, N> &rows) {
        for (int i = 0; i < height(); i++) { this[i] = row(rows[i], rows[i] * N * sizeof(T)); }
    }
    ColorBlock(const std::array<row, N> &rows) {
        for (int i = 0; i < height(); i++) { this[i] = rows[i]; }
    }

    ColorBlock(const T *pixels) {
        for (int i = 0; i < height(); i++) { _rows[i] = std::span(pixels[i * width()]); }
    }

    ColorBlock(const T *image, int imageWidth, int imageHeight, int x = 0, int y = 0) {
        int imageX = x * width();
        int imageY = y * height();

        assert(imageX > 0 && imageX + width() < imageWidth);
        assert(imageY > 0 && imageY + height() < imageHeight);

        T *start = &image[imageX + (imageY * imageWidth)];

        for (int i = 0; i < height(); i++) { _rows[i] = std::span(start[i * imageWidth]); }
    }

    constexpr T &operator[](size_t n) noexcept { return _rows[n]; }

    constexpr int width() noexcept { return N; }
    constexpr int height() noexcept { return M; }
    constexpr int size() noexcept { return N * M; }

   private:
    std::array<row, M> _rows;
};

using RGBABlock4x4 = ColorBlock<4, 4, Color32>;
using RBlock4x4 = ColorBlock<4, 4, uint8_t>;
