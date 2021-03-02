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

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <vector>

#include "Color.h"
#include "Vector4Int.h"
#include "ndebug.h"

namespace rgbcx {
template <typename S, size_t N> class RowView {
   public:
    RowView(S *start, int pixel_stride = 1) : start(start), pixel_stride(pixel_stride) {}

    constexpr S &operator[](size_t index) noexcept(ndebug) {
        assert(index < N);
        return start[index * pixel_stride];
    }
    constexpr const S &operator[](size_t index) const noexcept(ndebug) {
        assert(index < N);
        return start[index * pixel_stride];
    }

    constexpr int Size() noexcept { return N; }

    S *const start;
    const int pixel_stride;
};

template <typename S, size_t M, size_t N> class BlockView {
   public:
    using Row = RowView<S, N>;

    BlockView(S *start, int row_stride = N, int pixel_stride = 1) : start(start), row_stride(row_stride), pixel_stride(pixel_stride) {}

    constexpr Row operator[](unsigned index) noexcept(ndebug) {
        assert(index < M);
        return RowView<S, N>(&start[row_stride * (int)index], pixel_stride);
    }

    constexpr int Width() noexcept { return N; }
    constexpr int Height() noexcept { return M; }
    constexpr int Size() noexcept { return N * M; }

    constexpr S &Get(unsigned x, unsigned y) noexcept(ndebug) {
        assert(x < N);
        assert(y < M);
        return start[(row_stride * (int)y) + (pixel_stride * (int)x)];
    }

    constexpr S Get(unsigned x, unsigned y) const noexcept(ndebug) {
        assert(x < N);
        assert(y < M);
        return start[(row_stride * (int)y) + (pixel_stride * (int)x)];
    }

    constexpr void Set(unsigned x, unsigned y, S value) noexcept(ndebug) {
        assert(x < N);
        assert(y < M);
        start[(row_stride * (int)y) + (pixel_stride * (int)x)] = value;
    }

    constexpr S &Get(unsigned i) noexcept(ndebug) { return Get(i % N, i / N); }
    constexpr S Get(unsigned i) const noexcept(ndebug) { return Get(i % N, i / N); }

    constexpr std::array<S, M * N> Flatten() noexcept {
        std::array<S, M * N> result;
        for (unsigned x = 0; x < N; x++) {
            for (unsigned y = 0; y < M; y++) { result[x + (N * y)] = Get(x, y); }
        }
        return result;
    }

    S *const start;
    const int row_stride;
    const int pixel_stride;
};

template <size_t M, size_t N> class ColorBlockView : public BlockView<Color, M, N> {
   public:
    using Base = BlockView<Color, M, N>;
    using ChannelView = BlockView<uint8_t, M, N>;

    struct BlockMetrics {
        Color min;
        Color max;
        Color avg;
        bool is_greyscale;
        bool has_black;
        Vector4Int sums;
    };

    ColorBlockView(Color *start, int row_stride = N, int pixel_stride = 1) : Base(start, row_stride, pixel_stride) {}

    constexpr ChannelView GetChannel(uint8_t index) noexcept(ndebug) {
        assert(index < 4U);
        auto channelStart = reinterpret_cast<uint8_t *>(Base::start) + index;
        return ChannelView(channelStart, Base::row_stride * 4, Base::pixel_stride * 4);
    }

    void SetRGB(unsigned x, unsigned y, Color value) noexcept(ndebug) { Base::Get(x, y).SetRGB(value); }

    bool IsSingleColor() {
        auto first = Base::Get(0, 0);
        for (unsigned j = 1; j < M * N; j++) {
            if (Base::Get(j) != first) return false;
        }
        return true;
    }

    BlockMetrics GetMetrics(unsigned black_threshold = 4) {
        BlockMetrics metrics;
        metrics.min = Color(UINT8_MAX, UINT8_MAX, UINT8_MAX);
        metrics.max = Color(0, 0, 0);
        metrics.has_black = false;
        metrics.is_greyscale = true;
        metrics.sums = {0, 0, 0};

        for (unsigned i = 0; i < M * N; i++) {
            auto val = Base::Get(i);
            for (unsigned c = 0; c < 3; c++) {
                metrics.min[c] = std::min(metrics.min[c], val[c]);
                metrics.max[c] = std::max(metrics.max[c], val[c]);
                metrics.sums[c] += val[c];
            }
            metrics.is_greyscale &= ((val.r == val.g) && (val.r == val.b));
            metrics.has_black |= (val.r | val.g | val.b < black_threshold);
        }

        for (unsigned c = 0; c < 3; c++) { metrics.avg[c] = (uint8_t)((unsigned)metrics.sums[c] / (M * N)); }

        return metrics;
    }
};

using Color4x4 = ColorBlockView<4, 4>;
using Byte4x4 = BlockView<uint8_t, 4, 4>;

}  // namespace rgbcx