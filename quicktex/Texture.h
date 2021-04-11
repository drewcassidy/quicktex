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

namespace quicktex {

class Texture {
   public:
    virtual ~Texture() = default;

    virtual int Width() const { return _width; }
    virtual int Height() const { return _height; }
    virtual std::tuple<int, int> Size() const { return std::tuple<int, int>(_width, _height); }

    /**
     * The texture's total size
     * @return The size of the texture in bytes.
     */
    virtual size_t NBytes() const noexcept = 0;

    virtual const uint8_t *Data() const noexcept = 0;
    virtual uint8_t *Data() noexcept = 0;

   protected:
    Texture(int width, int height) : _width(width), _height(height) {
        if (width <= 0) throw std::invalid_argument("Texture width must be greater than 0");
        if (height <= 0) throw std::invalid_argument("Texture height must be greater than 0");
    }

    int _width;
    int _height;
};

class RawTexture : public Texture {
    using Base = Texture;

   public:
    /**
     * Create a new RawTexture
     * @param width width of the texture in pixels
     * @param height height of the texture in pixels
     */
    RawTexture(int width, int height) : Base(width, height), _pixels(_width * _height) {}

    Color GetPixel(int x, int y) const {
        if (x < 0 || x >= _width) throw std::invalid_argument("x value out of range.");
        if (y < 0 || y >= _height) throw std::invalid_argument("y value out of range.");
        return _pixels.at(x + (y * _width));
    }

    void SetPixel(int x, int y, Color val) {
        if (x < 0 || x >= _width) throw std::invalid_argument("x value out of range.");
        if (y < 0 || y >= _height) throw std::invalid_argument("y value out of range.");
        _pixels.at(x + (y * _width)) = val;
    }

    size_t NBytes() const noexcept override { return static_cast<unsigned long>(Width() * Height()) * sizeof(Color); }

    template <int N, int M> ColorBlock<N, M> GetBlock(int block_x, int block_y) const {
        if (block_x < 0) throw std::out_of_range("x value out of range.");
        if (block_y < 0) throw std::out_of_range("y value out of range.");

        // coordinates in the image of the top-left pixel of the selected block
        ColorBlock<N, M> block;
        int pixel_x = block_x * N;
        int pixel_y = block_y * M;

        if (pixel_x + N < _width && pixel_y + M < _height) {
            // fast memcpy if the block is entirely inside the bounds of the texture
            for (int y = 0; y < M; y++) {
                // copy each row into the ColorBlock
                block.SetRow(y, &_pixels[pixel_x + (_width * (pixel_y + y))]);
            }
        } else {
            // slower pixel-wise copy if the block goes over the edges
            for (int x = 0; x < N; x++) {
                for (int y = 0; y < M; y++) { block.Set(x, y, GetPixel((pixel_x + x) % _width, (pixel_y + y) % _height)); }
            }
        }

        return block;
    }

    template <int N, int M> void SetBlock(int block_x, int block_y, const ColorBlock<N, M> &block) {
        if (block_x < 0) throw std::out_of_range("x value out of range.");
        if (block_y < 0) throw std::out_of_range("y value out of range.");

        // coordinates in the image of the top-left pixel of the selected block
        int pixel_x = block_x * N;
        int pixel_y = block_y * M;

        if (pixel_x + N < _width && pixel_y + M < _height) {
            // fast row-wise memcpy if the block is entirely inside the bounds of the texture
            for (int y = 0; y < M; y++) {
                // copy each row out of the ColorBlock
                block.GetRow(y, &_pixels[pixel_x + (_width * (pixel_y + y))]);
            }
        } else {
            // slower pixel-wise copy if the block goes over the edges
            for (int x = 0; x < N; x++) {
                for (int y = 0; y < M; y++) { SetPixel((pixel_x + x) % _width, (pixel_y + y) % _height, block.Get(x, y)); }
            }
        }
    }

    virtual const uint8_t *Data() const noexcept override { return reinterpret_cast<const uint8_t *>(_pixels.data()); }
    virtual uint8_t *Data() noexcept override { return reinterpret_cast<uint8_t *>(_pixels.data()); }

   protected:
    std::vector<Color> _pixels;
};

template <typename B> class BlockTexture final : public Texture {
   private:
    std::vector<B> _blocks;
    int _width_b;
    int _height_b;

   public:
    using BlockType = B;
    using Base = Texture;

    /**
     * Create a new BlockTexture
     * @param width width of the texture in pixels. must be divisible by B::Width
     * @param height height of the texture in pixels. must be divisible by B::Height
     */
    BlockTexture(int width, int height) : Base(width, height) {
        _width_b = (_width + B::Width - 1) / B::Width;
        _height_b = (_height + B::Height - 1) / B::Height;
        _blocks = std::vector<B>(_width_b * _height_b);
    }

    constexpr int BlocksX() const { return _width_b; }
    constexpr int BlocksY() const { return _height_b; }
    constexpr std::tuple<int, int> BlocksXY() const { return std::tuple<int, int>(_width_b, _height_b); }

    B GetBlock(int x, int y) const {
        if (x < 0 || x >= _width_b) throw std::out_of_range("x value out of range.");
        if (y < 0 || y >= _height_b) throw std::out_of_range("y value out of range.");
        return _blocks.at(x + (y * _width_b));
    }

    void SetBlock(int x, int y, const B &val) {
        if (x < 0 || x >= _width_b) throw std::out_of_range("x value out of range.");
        if (y < 0 || y >= _height_b) throw std::out_of_range("y value out of range.");
        _blocks.at(x + (y * _width_b)) = val;
    }

    size_t NBytes() const noexcept override { return _blocks.size() * sizeof(B); }

    const uint8_t *Data() const noexcept override { return reinterpret_cast<const uint8_t *>(_blocks.data()); }
    uint8_t *Data() noexcept override { return reinterpret_cast<uint8_t *>(_blocks.data()); }
};

}  // namespace quicktex