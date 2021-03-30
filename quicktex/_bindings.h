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

#include <cstdint>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <type_traits>

#include "Block.h"
#include "Color.h"
#include "Texture.h"

namespace pybind11::detail {
template <> struct type_caster<quicktex::Color>;
}  // namespace pybind11::detail

namespace py = pybind11;
namespace quicktex::bindings {
using namespace pybind11::literals;

template <typename T> T BufferToTexture(py::buffer buf, int width, int height) {
    static_assert(std::is_base_of<Texture, T>::value);
    static_assert(std::is_constructible<T, int, int>::value);

    auto info = buf.request(false);
    auto output = T(width, height);  // std::make_shared<T>(width, height);
    auto dst_size = output.Size();

    if (info.format != py::format_descriptor<uint8_t>::format()) throw std::runtime_error("Incompatible format in python buffer: expected a byte array.");
    if (info.size < (ssize_t)dst_size) std::runtime_error("Incompatible format in python buffer: Input data is smaller than texture size.");
    if (info.ndim == 1) {
        if (info.shape[0] < (ssize_t)dst_size) throw std::runtime_error("Incompatible format in python buffer: 1-D buffer has incorrect length.");
        if (info.strides[0] != 1) throw std::runtime_error("Incompatible format in python buffer: 1-D buffer is not contiguous.");
    } else {
        throw std::runtime_error("Incompatible format in python buffer: Incorrect number of dimensions.");
    }

    std::memcpy(output.Data(), info.ptr, dst_size);

    return output;
}

inline int PyIndex(int val, int size, std::string name = "index") {
    if (val < -size || val >= size) throw std::out_of_range(name + " value out of range");
    if (val < 0) return size + val;
    return val;
}

template <typename T, typename Getter, typename Setter, typename Extent> void DefSubscript(py::class_<T> t, Getter&& get, Setter&& set, Extent&& ext) {
    using V = typename std::invoke_result<Getter, T*, int>::type;
    t.def(
        "__getitem__", [get, ext](T& self, int index) { return (self.*get)(PyIndex(index, (self.*ext)())); }, "key"_a);
    t.def(
        "__setitem__", [set, ext](T& self, int index, V val) { (self.*set)(PyIndex(index, (self.*ext)()), val); }, "key"_a, "value"_a);
}

template <typename T, typename Getter, typename Setter, typename Extent> void DefSubscript2D(py::class_<T> t, Getter&& get, Setter&& set, Extent&& ext) {
    using V = typename std::invoke_result<Getter, T*, int, int>::type;
    using Coords = std::tuple<int, int>;
    t.def(
        "__getitem__",
        [get, ext](T& self, Coords pnt) {
            Coords s = (self.*ext)();
            int x = PyIndex(std::get<0>(pnt), std::get<0>(s), "x");
            int y = PyIndex(std::get<1>(pnt), std::get<1>(s), "y");
            return (self.*get)(x, y);
        },
        "key"_a);
    t.def(
        "__setitem__",
        [set, ext](T& self, Coords pnt, V val) {
            Coords s = (self.*ext)();
            int x = PyIndex(std::get<0>(pnt), std::get<0>(s), "x");
            int y = PyIndex(std::get<1>(pnt), std::get<1>(s), "y");
            (self.*set)(x, y, val);
        },
        "key"_a, "value"_a);
}

template <typename B> py::class_<BlockTexture<B>> BindBlockTexture(py::module_& m, const char* name) {
    using BTex = BlockTexture<B>;

    py::type texture = py::type::of<Texture>();

    py::class_<BTex> block_texture(m, name, texture);

    block_texture.def(py::init<int, int>(), "width"_a, "height"_a);
    block_texture.def("get_block", &BTex::GetBlock, "x"_a, "y"_a);
    block_texture.def("set_block", &BTex::SetBlock, "x"_a, "y"_a, "block"_a);

    block_texture.def_static("from_bytes", &BufferToTexture<BTex>, "data"_a, "width"_a, "height"_a);

    block_texture.def_property_readonly("blocks_x", &BTex::BlocksX);
    block_texture.def_property_readonly("blocks_y", &BTex::BlocksY);
    block_texture.def_property_readonly("blocks_xy", &BTex::BlocksXY);

    DefSubscript2D(block_texture, &BTex::GetPixel, &BTex::SetPixel, &BTex::BlocksXY);
}
}  // namespace quicktex::bindings