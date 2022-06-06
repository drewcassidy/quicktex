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
#include <vector>

#include "Texture.h"

namespace quicktex {
template <typename B> class BlockTexture final : public Texture {
   private:
    std::vector<B> _blocks;
    unsigned _width_b;
    unsigned _height_b;

   public:
    using BlockType = B;
    using Base = Texture;

    /**
     * Create a new BlockTexture
     * @param width width of the texture in pixels. must be divisible by B::width
     * @param height height of the texture in pixels. must be divisible by B::height
     */
    BlockTexture(int width, int height) : Base(width, height) {
        _width_b = (width + B::Width - 1) / B::Width;
        _height_b = (height + B::Height - 1) / B::Height;
        _blocks = std::vector<B>(_width_b * _height_b);
    }

    constexpr unsigned bwidth() const { return _width_b; }
    constexpr unsigned bheight() const { return _height_b; }
    constexpr std::tuple<int, int> bsize() const { return std::tuple<int, int>(_width_b, _height_b); }

    B get_block(unsigned x, unsigned y) const {
        if (x >= _width_b) throw std::out_of_range("x value out of range.");
        if (y >= _height_b) throw std::out_of_range("y value out of range.");
        return _blocks.at(x + (y * _width_b));
    }

    void set_block(unsigned x, unsigned y, const B &val) {
        if (x >= _width_b) throw std::out_of_range("x value out of range.");
        if (y >= _height_b) throw std::out_of_range("y value out of range.");
        _blocks.at(x + (y * _width_b)) = val;
    }

    size_t nbytes() const noexcept override { return _blocks.size() * sizeof(B); }

    const uint8_t *data() const noexcept override { return reinterpret_cast<const uint8_t *>(_blocks.data()); }
    uint8_t *data() noexcept override { return reinterpret_cast<uint8_t *>(_blocks.data()); }
};
}  // namespace quicktex
