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

/*    using IType = Interpolator::Type;
    auto interpolator_type = py::enum_<IType>(s3tc, "InterpolatorType", R"pbdoc(
An enum representing various methods for interpolating colors, used by the BC1 and BC3 encoders/decoders.
Vendor-specific interpolation modes should only be used when the result will only be used on that type of GPU.
For most applications, :py:attr:`~quicktex.s3tc.InterpolatorType.Ideal` should be used.
)pbdoc");

    interpolator_type.value("Ideal", IType::Ideal, "The default mode, with no rounding for colors 2 and 3. This matches the D3D10 docs on BC1.");
    interpolator_type.value("IdealRound", IType::IdealRound, "Round colors 2 and 3. Matches the AMD Compressonator tool and the D3D9 docs on DXT1.");
    interpolator_type.value("Nvidia", IType::Nvidia, "Nvidia GPU mode.");
    interpolator_type.value("AMD", IType::AMD, "AMD GPU mode.");*/

    InitInterpolator(s3tc);
    InitBC1(s3tc);
    InitBC3(s3tc);
    InitBC4(s3tc);
    InitBC5(s3tc);
}
}  // namespace quicktex::bindings
