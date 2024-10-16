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

#include <pybind11/pybind11.h>

#include "interpolator/Interpolator.h"

namespace py = pybind11;
namespace quicktex::bindings {

using namespace quicktex;
using namespace quicktex::s3tc;

void InitInterpolator(py::module_ &s3tc);
void InitBC1(py::module_ &s3tc);
void InitBC3(py::module_ &s3tc);
void InitBC4(py::module_ &s3tc);
void InitBC5(py::module_ &s3tc);

void InitS3TC(py::module_ &m) {
    py::module_ s3tc = m.def_submodule("_s3tc", "s3tc compression library based on rgbcx.h written by Richard Goldreich");

    InitInterpolator(s3tc);
    InitBC1(s3tc);
    InitBC4(s3tc);
    InitBC3(s3tc);
    InitBC5(s3tc);
}
}  // namespace quicktex::bindings
