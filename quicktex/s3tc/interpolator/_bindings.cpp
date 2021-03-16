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

#include <array>

#include "Interpolator.h"

namespace py = pybind11;
namespace quicktex::bindings {

using namespace quicktex::s3tc;
using InterpolatorPtr = std::shared_ptr<Interpolator>;

void InitInterpolator(py::module_ &s3tc) {
    auto interpolator = s3tc.def_submodule("_interpolator", "Classes defining various methods of interpolating colors in BC1 and BC3 textures");

    // Interpolator
    py::class_<Interpolator> ideal(
        interpolator, "Interpolator",
        "Interpolator base class representing the default mode, with no rounding for colors 2 and 3. This matches the D3D10 docs on BC1.");

    // InterpolatorRound
    py::class_<InterpolatorRound> round(interpolator, "InterpolatorRound", ideal,
                                        "Round colors 2 and 3. Matches the AMD Compressonator tool and the D3D9 docs on DXT1.");

    // InterpolatorNvidia
    py::class_<InterpolatorNvidia> nvidia(interpolator, "InterpolatorNvidia", ideal, "Nvidia GPU mode.");

    // InterpolatorAMD
    py::class_<InterpolatorAMD> amd(interpolator, "InterpolatorAMD", ideal, "AMD GPU mode.");
}
}  // namespace quicktex::bindings