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

#include "_bindings.h"

#include <pybind11/pybind11.h>

#include "Decoder.h"
#include "Encoder.h"
#include "OldColor.h"
#include "_bindings.h"
#include "texture/RawTexture.h"
#include "texture/Texture.h"

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;

namespace quicktex::bindings {

void InitS3TC(py::module_ &m);

PYBIND11_MODULE(_quicktex, m) {
    m.doc() = "More Stuff";

#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif

#ifdef NDEBUG
    m.attr("_debug_build") = false;
#else
    m.attr("_debug_build") = true;
#endif

    py::options options;

    // Texture

    py::class_<Texture> texture(m, "Texture", py::buffer_protocol());

    texture.def_property_readonly("nbytes", &Texture::nbytes);
    texture.def_property_readonly("size", &Texture::Size);
    texture.def_readonly("width", &Texture::width);
    texture.def_readonly("height", &Texture::height);

    texture.def_buffer([](Texture &t) { return py::buffer_info(t.data(), t.nbytes()); });
    texture.def("tobytes",
                [](const Texture &t) { return py::bytes(reinterpret_cast<const char *>(t.data()), t.nbytes()); });

    // RawTexture

    py::class_<RawTexture, Texture> raw_texture(m, "RawTexture");

    raw_texture.def(py::init<int, int>(), "width"_a, "height"_a);
    raw_texture.def_static("frombytes", &BufferToTexture<RawTexture>, "data"_a, "width"_a, "height"_a);

    DefSubscript2DRef(
        raw_texture, [](RawTexture &self, int x, int y) -> Color { return self.pixel(x, y); },
        [](RawTexture &self, int x, int y, Color val) { self.pixel(x, y) = val; }, &RawTexture::Size);

    InitS3TC(m);
}

}  // namespace quicktex::bindings