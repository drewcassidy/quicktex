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

#include <pybind11/operators.h>
#include <pybind11/pybind11.h>

#include <cstdint>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <type_traits>

#include "Color.h"
#include "ColorBlock.h"
#include "Texture.h"
#include "util.h"

namespace pybind11::detail {
using namespace quicktex;
/// Type caster for color class to allow it to be converted to and from a python tuple
template <> struct type_caster<Color> {
   public:
    PYBIND11_TYPE_CASTER(Color, _("Color"));

    bool load(handle src, bool) {
        PyObject* source = src.ptr();

        PyObject* tmp = PySequence_Tuple(source);

        // if the object is not a tuple, return false
        if (!tmp) { return false; }  // incorrect type

        // check the size
        Py_ssize_t size = PyTuple_Size(tmp);
        if (size < 3 || size > 4) { return false; }  // incorrect size

        value.a = 0xFF;
        // now we get the contents
        for (int i = 0; i < size; i++) {
            PyObject* src_chan = PyTuple_GetItem(tmp, i);
            PyObject* tmp_chan = PyNumber_Long(src_chan);

            if (!tmp_chan) return false;  // incorrect channel type

            auto chan = PyLong_AsLong(tmp_chan);
            if (chan > 0xFF || chan < 0) return false;  // item out of range
            value[static_cast<unsigned>(i)] = static_cast<uint8_t>(chan);
            Py_DECREF(tmp_chan);
        }
        Py_DECREF(tmp);

        return !PyErr_Occurred();
    }

    static handle cast(Color src, return_value_policy, handle) {
        PyObject* val = PyTuple_New(4);

        for (int i = 0; i < 4; i++) {
            PyObject* chan = PyLong_FromLong(src[static_cast<unsigned>(i)]);
            PyTuple_SetItem(val, i, chan);
        }

        return val;
    }
};
}  // namespace pybind11::detail

namespace py = pybind11;
namespace quicktex::bindings {
using namespace pybind11::literals;

template <typename T> T BufferToTexture(py::buffer buf, int width, int height) {
    static_assert(std::is_base_of<Texture, T>::value);
    static_assert(std::is_constructible<T, int, int>::value);

    auto info = buf.request(false);
    auto output = T(width, height);
    auto dst_size = output.NBytes();

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

template <typename T> T BufferToPOD(py::buffer buf) {
    static_assert(std::is_trivially_copyable_v<T>);

    auto info = buf.request(false);

    if (info.format != py::format_descriptor<uint8_t>::format()) throw std::runtime_error("Incompatible format in python buffer: expected a byte array.");
    if (info.size < (ssize_t)sizeof(T)) std::runtime_error("Incompatible format in python buffer: Input data is smaller than texture size.");
    if (info.ndim == 1) {
        if (info.shape[0] < (ssize_t)sizeof(T)) throw std::runtime_error("Incompatible format in python buffer: 1-D buffer has incorrect length.");
        if (info.strides[0] != 1) throw std::runtime_error("Incompatible format in python buffer: 1-D buffer is not contiguous.");
    } else {
        throw std::runtime_error("Incompatible format in python buffer: Incorrect number of dimensions.");
    }

    const T* ptr = reinterpret_cast<const T*>(info.ptr);
    return *ptr;
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

template <typename Tpy, typename Getter, typename Setter, typename Extent> void DefSubscript2D(Tpy t, Getter&& get, Setter&& set, Extent&& ext) {
    using T = typename Tpy::type;
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
        [set, ext](T& self, Coords pnt, const V& val) {
            Coords s = (self.*ext)();
            int x = PyIndex(std::get<0>(pnt), std::get<0>(s), "x");
            int y = PyIndex(std::get<1>(pnt), std::get<1>(s), "y");
            (self.*set)(x, y, val);
        },
        "key"_a, "value"_a);
}

template <typename B> py::class_<B> BindBlock(py::module_& m, const char* name) {
    const char* frombytes_doc = R"doc(
        Create a new {0} by copying a bytes-like object.

        :param b: A bytes-like object at least the size of the block.
    )doc";

    const char* tobytes_doc = R"doc(
        Pack the {0} into a bytestring.

        :returns: A bytes object of length {1}.
    )doc";

    py::class_<B> block(m, name, py::buffer_protocol());
    block.def_static("frombytes", &BufferToPOD<B>, "data"_a, Format(frombytes_doc, name).c_str());

    block.def_readonly_static("width", &B::Width, "The width of the block in pixels.");
    block.def_readonly_static("height", &B::Height, "The height of the block in pixels.");
    block.def_property_readonly_static(
        "size", [](py::object) { return std::make_tuple(B::Width, B::Height); }, "The dimensions of the block in pixels.");
    block.def_property_readonly_static(
        "nbytes", [](py::object) { return sizeof(B); }, "The size of the block in bytes.");

    block.def(py::self == py::self);

    block.def_buffer([](B& b) { return py::buffer_info(reinterpret_cast<uint8_t*>(&b), sizeof(B)); });
    block.def(
        "tobytes", [](const B& b) { return py::bytes(reinterpret_cast<const char*>(&b), sizeof(B)); },
        Format(tobytes_doc, name, std::to_string(sizeof(B))).c_str());

    return std::move(block);
}

template <typename B> py::class_<BlockTexture<B>> BindBlockTexture(py::module_& m, const char* name) {
    const auto* const constructor_str = R"doc(
        Create a new blank {0} with the given dimensions.
        If the dimenions are not multiples of the block dimensions, enough blocks will be allocated
        to cover the entire texture, and it will be implicitly cropped during decoding.

        :param int width: The width of the texture in pixels. Must be > 0.
        :param int height: The height of the texture in pixels. must be > 0
        )doc";

    const auto* const from_bytes_str = R"doc(
        Create a new {0} with the given dimensions, and copy a bytes-like object into it.
        If the dimenions are not multiples of the block dimensions, enough blocks will be allocated
        to cover the entire texture, and it will be implicitly cropped during decoding.

        :param b: A bytes-like object at least the size of the resulting texture.
        :param int width: The width of the texture in pixels. Must be > 0.
        :param int height: The height of the texture in pixels. must be > 0
    )doc";

    using BTex = BlockTexture<B>;

    py::class_<BTex, Texture> block_texture(m, name);

    block_texture.def(py::init<int, int>(), "width"_a, "height"_a, Format(constructor_str, name).c_str());
    block_texture.def_static("from_bytes", &BufferToTexture<BTex>, "data"_a, "width"_a, "height"_a, Format(from_bytes_str, name).c_str());

    block_texture.def_property_readonly("width_blocks", &BTex::BlocksX, "The width of the texture in blocks.");
    block_texture.def_property_readonly("height_blocks", &BTex::BlocksY, "The height of the texture in blocks.");
    block_texture.def_property_readonly("size_blocks", &BTex::BlocksXY, "The dimensions of the texture in blocks.");

    DefSubscript2D(block_texture, &BTex::GetBlock, &BTex::SetBlock, &BTex::BlocksXY);

    return std::move(block_texture);
}
}  // namespace quicktex::bindings