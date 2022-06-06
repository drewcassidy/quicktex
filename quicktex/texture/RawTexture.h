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
#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <vector>

#include "Color.h"
#include "ColorBlock.h"
#include "OldColor.h"
#include "texture/Texture.h"

namespace quicktex {
class RawTexture : public Texture {
    using Base = Texture;

   public:
    /**
     * Create a new RawTexture
     * @param width width of the texture in pixels
     * @param height height of the texture in pixels
     */
    RawTexture(int width, int height) : Base(width, height), _pixels(width * height) {}

    quicktex::Color pixel(unsigned x, unsigned y) const;

    quicktex::Color &pixel(unsigned x, unsigned y);

    quicktex::Color pixel_wrapped(unsigned x, unsigned y) const { return pixel(x % width, y % height); }

    quicktex::Color &pixel_wrapped(unsigned x, unsigned y) { return pixel(x % width, y % height); }

    size_t nbytes() const noexcept override { return static_cast<size_t>(width * height) * sizeof(quicktex::Color); }

    template <int N, int M> quicktex::ColorBlock<N, M> get_block(int block_x, int block_y) const {
        if (block_x < 0) throw std::out_of_range("x value out of range.");
        if (block_y < 0) throw std::out_of_range("y value out of range.");

        // coordinates in the image of the top-left pixel of the selected block
        quicktex::ColorBlock<N, M> block;
        int pixel_x = block_x * N;
        int pixel_y = block_y * M;

        // slower pixel-wise copy if the block goes over the edges
        for (int x = 0; x < N; x++) {
            for (int y = 0; y < M; y++) { block.Set(x, y, pixel((pixel_x + x) % width, (pixel_y + y) % height)); }
        }

        return block;
    }

    template <int N, int M> void set_block(int block_x, int block_y, const quicktex::ColorBlock<N, M> &block) {
        if (block_x < 0) throw std::out_of_range("x value out of range.");
        if (block_y < 0) throw std::out_of_range("y value out of range.");

        // coordinates in the image of the top-left pixel of the selected block
        int pixel_x = block_x * N;
        int pixel_y = block_y * M;

        // slower pixel-wise copy if the block goes over the edges
        for (int x = 0; x < N; x++) {
            for (int y = 0; y < M; y++) { pixel((pixel_x + x) % width, (pixel_y + y) % height) = block.Get(x, y); }
        }
    }

    virtual const uint8_t *data() const noexcept override { return reinterpret_cast<const uint8_t *>(_pixels.data()); }
    virtual uint8_t *data() noexcept override { return reinterpret_cast<uint8_t *>(_pixels.data()); }

   protected:
    std::vector<quicktex::Color> _pixels;
};
}  // namespace quicktex