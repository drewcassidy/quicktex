/*  Quicktex Texture Compression Library
    Copyright (C) 2021 Andrew Cassidy <drewcassidy@me.com>
    Partially derived from rgbcx.h written by Richard Geldreich <richgel99@gmail.com> and licenced under the public domain

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
#include <memory>

#include "Interpolator.h"

namespace py = pybind11;
namespace quicktex::bindings {

using namespace quicktex::s3tc;
using InterpolatorPtr = std::shared_ptr<Interpolator>;

void InitInterpolator(py::module_ &s3tc) {
    auto interpolator = s3tc.def_submodule("_interpolator", "internal interpolator module");

    // Interpolator
    py::class_<Interpolator, std::shared_ptr<Interpolator>> ideal(
        interpolator, "Interpolator", R"doc(
        Interpolator base class representing the ideal interpolation mode, with no rounding for colors 2 and 3.
        This matches the `D3D10 docs`_ on BC1.

        .. _D3D10 docs: https://docs.microsoft.com/en-us/windows/win32/direct3d10/d3d10-graphics-programming-guide-resources-block-compression
    )doc");

    // InterpolatorRound
    py::class_<InterpolatorRound, std::shared_ptr<InterpolatorRound>> round(interpolator, "InterpolatorRound", ideal, R"doc(
        Base: :py:class:`~quicktex.s3tc.interpolator.Interpolator`

        Interpolator class representing the ideal rounding interpolation mode.
        Round colors 2 and 3. Matches the AMD Compressonator tool and the `D3D9 docs`_ on DXT1.

        .. _D3D9 docs: https://docs.microsoft.com/en-us/windows/win32/direct3d9/opaque-and-1-bit-alpha-textures
    )doc");

    // InterpolatorNvidia
    py::class_<InterpolatorNvidia, std::shared_ptr<InterpolatorNvidia>> nvidia(interpolator, "InterpolatorNvidia", ideal, R"doc(
        Base: :py:class:`~quicktex.s3tc.interpolator.Interpolator`

        Interpolator class representing the Nvidia GPU interpolation mode.
    )doc");

    // InterpolatorAMD
    py::class_<InterpolatorAMD, std::shared_ptr<InterpolatorAMD>> amd(interpolator, "InterpolatorAMD", ideal, R"doc(
        Base: :py:class:`~quicktex.s3tc.interpolator.Interpolator`

        Interpolator class representing the AMD GPU interpolation mode.
    )doc");
}
}  // namespace quicktex::bindings