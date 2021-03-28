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

#include "_bindings.h"

#include <pybind11/pybind11.h>

#include "Color.h"
#include "Decoder.h"
#include "Encoder.h"
#include "Texture.h"
#include "_bindings.h"

namespace py = pybind11;

namespace pybind11::detail {
using namespace quicktex;
/// Type caster for color class to allow it to be converted to and from a python tuple
template <> struct type_caster<Color> {
   public:
    PYBIND11_TYPE_CASTER(Color, _("Color"));

    bool load(handle src, bool) {
        PyObject *source = src.ptr();

        PyObject *tmp = PySequence_Tuple(source);

        // if the object is not a tuple, return false
        if (!tmp) { return false; }  // incorrect type

        // check the size
        Py_ssize_t size = PyTuple_Size(tmp);
        if (size < 3 || size > 4) { return false; }  // incorrect size

        value.a = 0xFF;
        // now we get the contents
        for (int i = 0; i < size; i++) {
            PyObject *src_chan = PyTuple_GetItem(tmp, i);
            PyObject *tmp_chan = PyNumber_Long(src_chan);

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
        PyObject *val = PyTuple_New(4);

        for (int i = 0; i < 4; i++) {
            PyObject *chan = PyLong_FromLong(src[static_cast<unsigned>(i)]);
            PyTuple_SetItem(val, i, chan);
        }

        return val;
    }
};
}  // namespace pybind11::detail

namespace quicktex::bindings {

void InitS3TC(py::module_ &m);

/*py::bytes EncodeImage(const BlockEncoder &self, py::bytes decoded, unsigned image_width, unsigned image_height) {
    if (image_width % self.BlockWidth() != 0) throw std::invalid_argument("Width is not an even multiple of block_width");
    if (image_height % self.BlockHeight() != 0) throw std::invalid_argument("Height is not an even multiple of block_height");
    if (image_width == 0 || image_height == 0) throw std::invalid_argument("Image has zero size");

    size_t size = image_width * image_height;
    size_t block_size = (size / (self.BlockHeight() * self.BlockWidth())) * self.BlockSize();
    size_t color_size = size * sizeof(Color);

    std::string encoded_str = std::string(block_size, 0);
    std::string decoded_str = (std::string)decoded;  // decoded data is copied here, unfortunately

    if (decoded_str.size() != color_size) throw std::invalid_argument("Incompatible data: image width and height do not match the size of the decoded image");

    self.EncodeImage(reinterpret_cast<uint8_t *>(encoded_str.data()), reinterpret_cast<Color *>(decoded_str.data()), image_width, image_height);

    auto bytes = py::bytes(encoded_str);  // encoded data is copied here, unfortunately

    return bytes;
}

py::bytes DecodeImage(const BlockDecoder &self, py::bytes encoded, unsigned image_width, unsigned image_height) {
    if (image_width % self.WidthInBlocks() != 0) throw std::invalid_argument("Width is not an even multiple of block_width");
    if (image_height % self.HeightInBlocks() != 0) throw std::invalid_argument("Height is not an even multiple of block_height");
    if (image_width == 0 || image_height == 0) throw std::invalid_argument("Image has zero size");

    size_t size = image_width * image_height;
    size_t block_size = (size / (self.HeightInBlocks() * self.WidthInBlocks())) * self.BlockSize();
    size_t color_size = size * sizeof(Color);

    std::string encoded_str = (std::string)encoded;  // encoded data is copied here, unfortunately
    std::string decoded_str = std::string(color_size, 0);

    if (encoded_str.size() != block_size) throw std::invalid_argument("Incompatible data: image width and height do not match the size of the encoded image");

    self.DecodeImage(reinterpret_cast<uint8_t *>(encoded_str.data()), reinterpret_cast<Color *>(decoded_str.data()), image_width, image_height);

    auto bytes = py::bytes(decoded_str);  // decoded data is copied here, unfortunately

    return bytes;
}*/

PYBIND11_MODULE(_quicktex, m) {
    m.doc() = "More Stuff";

    py::options options;

    py::class_<Texture> texture(m, "Texture");

    texture.def_property_readonly("size", &Texture::Size);
    texture.def_property_readonly("dimensions", &Texture::Dimensions);
    texture.def_property_readonly("width", &Texture::Width);
    texture.def_property_readonly("height", &Texture::Height);

    py::class_<RawTexture> raw_texture(m, "RawTexture", texture);

    raw_texture.def(py::init<int, int>(), "width"_a, "height"_a);
    raw_texture.def("get_pixel", &RawTexture::GetPixel);
    raw_texture.def("set_pixel", &RawTexture::SetPixel);

    InitS3TC(m);
}

}  // namespace quicktex::bindings