/*  Quicktex Texture Compression Library
    Copyright (C) 2021-2024 Andrew Cassidy <drewcassidy@me.com>
    Partially derived from rgbcx.h written by Richard Geldreich <richgel99@gmail.com>
    and licenced under the public domain

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */

#include "_bindings.h"

#include <pybind11/pybind11.h>

#include "Color.h"
#include "Decoder.h"
#include "Encoder.h"
#include "Texture.h"
#include "_bindings.h"

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

    py::options options;

    // Texture

    py::class_<Texture> texture(m, "Texture", py::buffer_protocol());

    texture.def_property_readonly("nbytes", &Texture::NBytes);
    texture.def_property_readonly("size", &Texture::Size);
    texture.def_property_readonly("width", &Texture::Width);
    texture.def_property_readonly("height", &Texture::Height);

    texture.def_buffer([](Texture &t) { return py::buffer_info(t.Data(), t.NBytes()); });
    texture.def("tobytes", [](const Texture &t) { return py::bytes(reinterpret_cast<const char *>(t.Data()), t.NBytes()); });

    // RawTexture

    py::class_<RawTexture, Texture> raw_texture(m, "RawTexture");

    raw_texture.def(py::init<int, int>(), "width"_a, "height"_a);
    raw_texture.def_static("frombytes", &BufferToTexture<RawTexture>, "data"_a, "width"_a, "height"_a);

    DefSubscript2D(raw_texture, &RawTexture::GetPixel, &RawTexture::SetPixel, &RawTexture::Size);

    InitS3TC(m);
}

}  // namespace quicktex::bindings