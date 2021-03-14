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

#include <pybind11/pybind11.h>

#include "Interpolator.h"

namespace py = pybind11;
namespace quicktex::bindings {

using namespace quicktex;
using namespace quicktex::s3tc;

void InitBC1(py::module_ &s3tc);
void InitBC3(py::module_ &s3tc);
void InitBC4(py::module_ &s3tc);
void InitBC5(py::module_ &s3tc);

void InitS3TC(py::module_ &m) {
    py::module_ s3tc = m.def_submodule("_s3tc", "s3tc compression library based on rgbcx.h written by Richard Goldreich");

    using IType = Interpolator::Type;
    py::enum_<IType>(s3tc, "InterpolatorType")
        .value("Ideal", IType::Ideal)
        .value("IdealRound", IType::IdealRound)
        .value("Nvidia", IType::Nvidia)
        .value("AMD", IType::AMD);

    InitBC1(s3tc);
    InitBC3(s3tc);
    InitBC4(s3tc);
    InitBC5(s3tc);
}
}  // namespace quicktex::bindings
