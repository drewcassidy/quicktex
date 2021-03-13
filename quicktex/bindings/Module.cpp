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

#include "../Interpolator.h"

namespace py = pybind11;
namespace quicktex::bindings {

void InitEncoders(py::module_ &m);
void InitDecoders(py::module_ &m);

PYBIND11_MODULE(_quicktex, m) {
    m.doc() = "More Stuff";

    using IType = Interpolator::Type;
    py::enum_<IType>(m, "InterpolatorType")
        .value("Ideal", IType::Ideal)
        .value("IdealRound", IType::IdealRound)
        .value("Nvidia", IType::Nvidia)
        .value("AMD", IType::AMD);

    InitEncoders(m);
    InitDecoders(m);
}

}  // namespace quicktex::bindings