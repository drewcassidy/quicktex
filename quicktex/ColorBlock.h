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

#include <algorithm>
#include <array>
#include <climits>
#include <cstring>
#include <stdexcept>

#include "Color.h"
#include "Vector4Int.h"

namespace quicktex {
using Coords = std::tuple<int, int>;

template <int N, int M> class ColorBlock  {
   public:
    struct Metrics {
        Color min;
        Color max;
        Color avg;
        bool is_greyscale;
        bool has_black;
        Vector4Int sums;
    };

    static constexpr int Width = N;
    static constexpr int Height = M;

    constexpr Color Get(int x, int y) const {
        if (x >= Width || x < 0) throw std::invalid_argument("x value out of range");
        if (y >= Height || y < 0) throw std::invalid_argument("y value out of range");

        return _pixels[x + (N * y)];
    }

    constexpr Color Get(int i) const {
        if (i >= N * M || i < 0) throw std::invalid_argument("i value out of range");
        return _pixels[i];
    }

    void Set(int x, int y, const Color &value) {
        if (x >= Width || x < 0) throw std::invalid_argument("x value out of range");
        if (y >= Height || y < 0) throw std::invalid_argument("y value out of range");
        _pixels[x + (N * y)] = value;
    }

    void Set(int i, const Color &value) {
        if (i >= N * M || i < 0) throw std::invalid_argument("i value out of range");
        _pixels[i] = value;
    }

    void GetRow(int y, Color *dst) const {
        if (y >= Height || y < 0) throw std::invalid_argument("y value out of range");
        std::memcpy(dst, &_pixels[N * y], N * sizeof(Color));
    }

    void SetRow(int y, const Color *src) {
        if (y >= Height || y < 0) throw std::invalid_argument("y value out of range");
        std::memcpy(&_pixels[N * y], src, N * sizeof(Color));
    }

    bool IsSingleColor() const {
        auto first = Get(0, 0);
        for (unsigned j = 1; j < M * N; j++) {
            if (Get(j) != first) return false;
        }
        return true;
    }

    Metrics GetMetrics(bool ignore_black = false) const {
        Metrics metrics;
        metrics.min = Color(UINT8_MAX, UINT8_MAX, UINT8_MAX);
        metrics.max = Color(0, 0, 0);
        metrics.has_black = false;
        metrics.is_greyscale = true;
        metrics.sums = {0, 0, 0};

        unsigned total = 0;

        for (unsigned i = 0; i < M * N; i++) {
            Color val = Get(i);
            bool is_black = val.IsBlack();

            metrics.has_black |= is_black;

            if (ignore_black && is_black) { continue; }

            metrics.is_greyscale &= val.IsGrayscale();
            for (unsigned c = 0; c < 3; c++) {
                metrics.min[c] = std::min(metrics.min[c], val[c]);
                metrics.max[c] = std::max(metrics.max[c], val[c]);
                metrics.sums[c] += val[c];
            }
            total++;
        }

        if (total > 0) metrics.avg = (metrics.sums + Vector4Int(total / 2)) / (int)total;  // half-total added for better rounding
        return metrics;
    }

   private:
    std::array<Color, N * M> _pixels;
};

}  // namespace quicktex