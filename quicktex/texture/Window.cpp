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

#include "Window.h"

#include "texture/RawTexture.h"

namespace quicktex {

// Window
Window::Window(RawTexture& texture, unsigned int width, unsigned int height, unsigned int x, unsigned int y)
    : width(width), height(height), x(x), y(y), _texture(texture) {
    assert(x < texture.width);
    assert(y < texture.height);
}

Color& Window::pixel(unsigned px, unsigned py) {
    assert(px < width && py < height);
    return _texture.pixel(x + px, y + py);
}

Color Window::pixel(unsigned px, unsigned py) const {
    assert(px < width && py < height);
    return _texture.pixel(x + px, y + py);
}

WindowIterator Window::begin() { return WindowIterator(*this, 0, 0); }
WindowIterator Window::end() { return WindowIterator(*this, 0, height); }
WindowIterator Window::row_begin(unsigned int row) { return WindowIterator(*this, 0, row); }
WindowIterator Window::row_end(unsigned int row) { return WindowIterator(*this, 0, row + 1); }

bool Window::operator==(const Window& rhs) const {
    return width == rhs.width && height == rhs.height && x == rhs.x && y == rhs.y && &_texture == &rhs._texture;
}

// WindowIterator

WindowIterator::WindowIterator(Window& view, unsigned int x, unsigned int y) : x(x), y(y), _view(&view) {
    assert(x < view.width);
    assert(y < view.height || (y == view.height && x == 0));
    // if y == the height, and x == 0, then this is a sentinel for the end of iteration, and cannot be dereferenced
}

WindowIterator& quicktex::WindowIterator::operator++() {  // prefix increment
    x++;
    if (x >= _view->width) {
        x = 0;
        y++;
    }
    return *this;
}

WindowIterator WindowIterator::operator++(int) {  // postfix increment
    WindowIterator old = *this;
    ++(*this);
    return old;
}

Color& WindowIterator::operator*() const {  // dereference operator
    assert(_view != nullptr);
    assert(x < _view->width && y < _view->height);
    return _view->pixel(x, y);
}

Color* WindowIterator::operator->() { return &(**this); }  // returns a pointer to what's returned by operator*

bool WindowIterator::operator==(const WindowIterator& rhs) const {
    return x == rhs.x && y == rhs.y && _view == rhs._view;
}

static_assert(std::forward_iterator<WindowIterator>);
static_assert(sized_range<Window>);

}  // namespace quicktex
