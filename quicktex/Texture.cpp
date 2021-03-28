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
#include "Texture.h"

#include <cstdint>
#include <stdexcept>

#include "Color.h"

namespace quicktex {
RawTexture::RawTexture(int width, int height) : Base(width, height) { _pixels = new Color[(size_t)(_width * _height)]; }

RawTexture::RawTexture(RawTexture&& other) : Base(other._width, other._height) {
    _pixels = other._pixels;
    other._pixels = nullptr;
}

RawTexture::RawTexture(const RawTexture& other) : RawTexture(other._width, other._height) {
    std::memcpy(_pixels, other._pixels, (size_t)(_width * _height) * sizeof(Color));
}

RawTexture& RawTexture::operator=(RawTexture other) noexcept {
    swap(*this, other);
    return *this;
}

void swap(RawTexture& first, RawTexture& second) noexcept {
    using std::swap;  // enable ADL
    swap(first._pixels, second._pixels);
    swap(first._width, second._width);
    swap(first._height, second._height);
}

Color RawTexture::GetPixel(int x, int y) const {
    if (x < 0 || x >= _width) throw std::invalid_argument("x value out of range.");
    if (y < 0 || y >= _height) throw std::invalid_argument("y value out of range.");
    return _pixels[x + (y * _width)];
}

void RawTexture::SetPixel(int x, int y, Color val) {
    if (x < 0 || x >= _width) throw std::invalid_argument("x value out of range.");
    if (y < 0 || y >= _height) throw std::invalid_argument("y value out of range.");
    _pixels[x + (y * _width)] = val;
}
}  // namespace quicktex
