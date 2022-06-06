/*  Quicktex Texture Compression Library
    Copyright (C) 2021-2022 Andrew Cassidy <drewcassidy@me.com>
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
#include "Window.h"

namespace quicktex {

class Texture {
   public:
    const unsigned width;
    const unsigned height;

    virtual ~Texture() = default;

    virtual std::tuple<unsigned, unsigned> Size() const { return {width, height}; }

    /**
     * The texture's total size
     * @return The size of the texture in bytes.
     */
    virtual size_t nbytes() const noexcept = 0;

    virtual const uint8_t *data() const noexcept = 0;
    virtual uint8_t *data() noexcept = 0;

   protected:
    Texture(unsigned width, unsigned height) : width(width), height(height) {}
};



}  // namespace quicktex