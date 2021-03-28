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

#include <pybind11/pybind11.h>

#include <memory>
#include <stdexcept>

#include "Block.h"
#include "Color.h"
#include "Texture.h"

namespace pybind11::detail {
template <> struct type_caster<quicktex::Color>;
}  // namespace pybind11::detail

namespace py = pybind11;
namespace quicktex::bindings {

using namespace pybind11::literals;

template <typename B> py::class_<BlockTexture<B>> BindBlockTexture(py::module_& m, const char* name) {
    using BTex = BlockTexture<B>;

    py::type texture = py::type::of<Texture>();

    py::class_<BTex> block_texture(m, name, texture);

    block_texture.def(py::init<int, int>(), "width"_a, "height"_a);
    block_texture.def("get_block", &BTex::GetBlock, "x"_a, "y"_a);
    block_texture.def("set_block", &BTex::SetBlock, "x"_a, "y"_a, "block"_a);

    block_texture.def_property_readonly("blocks_x", &BTex::BlocksX);
    block_texture.def_property_readonly("blocks_y", &BTex::BlocksY);
}
}  // namespace quicktex::bindings