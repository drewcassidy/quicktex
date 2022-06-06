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
#include "Color.h"
#include "util/ranges.h"

namespace quicktex {

// forward declarations
class WindowIterator;
class RawTexture;

/**
 * Class representing a window into a RawTexture
 */
class Window {
   public:
    typedef Color value_type;

    const unsigned width, height;
    const unsigned x, y;

    Window(RawTexture &texture, unsigned width, unsigned height, unsigned x, unsigned y);

    Color &pixel(unsigned px, unsigned py);
    Color pixel(unsigned px, unsigned py) const;

    WindowIterator begin();
    WindowIterator end();
    WindowIterator row_begin(unsigned row);
    WindowIterator row_end(unsigned row);

    size_t size() const { return width * height; }

    bool operator==(const Window &rhs) const;

   private:
    RawTexture &_texture;
};

/**
 * Iterator returned by Window
 */
class WindowIterator {
   public:
    typedef long long difference_type;
    typedef Color value_type;

    unsigned x, y;

    WindowIterator(Window &view, unsigned x, unsigned y);
    WindowIterator() : x(0), y(0), _view(nullptr) {}

    Color &operator*() const;  // dereference
    Color *operator->();       // member access

    WindowIterator &operator++();    // prefix increment
    WindowIterator operator++(int);  // postfix increment
    bool operator==(const WindowIterator &rhs) const;

   private:
    Window *_view;
};

}  // namespace quicktex