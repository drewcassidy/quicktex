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

    constexpr int size() noexcept { return N; }

    S *const start;
    const int pixel_stride;
};

template <typename S, size_t M, size_t N> class BlockView {
   public:
    using Row = RowView<S, N>;

    BlockView(S *start, int row_stride = N, int pixel_stride = 1) : start(start), row_stride(row_stride), pixel_stride(pixel_stride) {}

    constexpr Row operator[](size_t index) noexcept(ndebug) {
        assert(index < M);
        return RowView<S, N>(&start[index * row_stride], pixel_stride);
    }

    constexpr int width() noexcept { return N; }
    constexpr int height() noexcept { return M; }
    constexpr int size() noexcept { return N * M; }

    constexpr S &get(unsigned x, unsigned y) noexcept(ndebug) {
        assert(x < N);
        assert(y < M);
        return start[(row_stride * y) + (pixel_stride * x)];
    }

    constexpr S get(unsigned x, unsigned y) const noexcept(ndebug) {
        assert(x < N);
        assert(y < M);
        return start[(row_stride * y) + (pixel_stride * x)];
    }

    constexpr void set(unsigned x, unsigned y, S value) noexcept(ndebug) {
        assert(x < N);
        assert(y < M);
        start[(row_stride * y) + (pixel_stride * x)] = value;
    }

    constexpr std::array<S, M * N> flatten() noexcept {
        std::array<S, M * N> result;
        for (unsigned x = 0; x < N; x++) {
            for (unsigned y = 0; y < M; y++) { result[x + (N * y)] = start[(row_stride * y) + (pixel_stride * x)]; }
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

    ColorBlockView(Color *start, int row_stride = N, int pixel_stride = 1) : Base(start, row_stride, pixel_stride) {}

    constexpr ChannelView GetChannel(uint8_t index) noexcept(ndebug) {
        assert(index < 4U);
        auto channelStart = reinterpret_cast<uint8_t *>(Base::start) + index;
        return ChannelView(channelStart, Base::row_stride * 4, Base::pixel_stride * 4);
    }

    constexpr ChannelView GetR() noexcept(ndebug) { return GetChannel(0); };
    constexpr ChannelView GetG() noexcept(ndebug) { return GetChannel(1); };
    constexpr ChannelView GetB() noexcept(ndebug) { return GetChannel(2); };
    constexpr ChannelView GetA() noexcept(ndebug) { return GetChannel(3); };
};

using Color4x4 = ColorBlockView<4, 4>;

}  // namespace rgbcx