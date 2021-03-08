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

#include "../BlockEncoder.h"
#include "../bitwiseEnums.h"
#include "../BC1/BC1Encoder.h"

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;
namespace rgbcx::bindings {

std::unique_ptr<BC1Encoder> MakeBC1Encoder(Interpolator::Type interpolator, unsigned level, bool use_3color, bool use_3color_black) {
    auto interpolator_ptr = (std::shared_ptr<Interpolator>)Interpolator::MakeInterpolator(interpolator);
    return std::make_unique<BC1Encoder>(interpolator_ptr, level, use_3color, use_3color_black);
}

void InitBC1(py::module_ &m) {
    auto block_encoder = py::type::of<BlockEncoder>();
    py::class_<BC1Encoder> bc1_encoder(m, "BC1Encoder", block_encoder);

    bc1_encoder.def(py::init(&MakeBC1Encoder), py::arg("interpolator") = Interpolator::Type::Ideal, py::arg("level") = 5, py::arg("use_3color") = true,
                    py::arg("use_3color_black") = true);
    bc1_encoder.def("set_level", &BC1Encoder::SetLevel);
    bc1_encoder.def_property("flags", &BC1Encoder::GetFlags, &BC1Encoder::SetFlags);
    bc1_encoder.def_property("error_mode", &BC1Encoder::GetErrorMode, &BC1Encoder::SetErrorMode);
    bc1_encoder.def_property("endpoint_mode", &BC1Encoder::GetEndpointMode, &BC1Encoder::SetEndpointMode);
    bc1_encoder.def_property("search_rounds", &BC1Encoder::GetSearchRounds, &BC1Encoder::SetSearchRounds);
    bc1_encoder.def_property("orderings_4", &BC1Encoder::GetOrderings4, &BC1Encoder::SetOrderings4);
    bc1_encoder.def_property("orderings_3", &BC1Encoder::GetOrderings3, &BC1Encoder::SetOrderings3);

    using Flags = BC1Encoder::Flags;
    py::enum_<Flags>(bc1_encoder, "Flags", py::arithmetic())
        .value("UseLikelyTotalOrderings", Flags::UseLikelyTotalOrderings)
        .value("TwoLeastSquaresPasses", Flags::TwoLeastSquaresPasses)
        .value("Use3ColorBlocksForBlackPixels", Flags::Use3ColorBlocksForBlackPixels)
        .value("Use3ColorBlocks", Flags::Use3ColorBlocks)
        .value("Iterative", Flags::Iterative)
        .value("Use6PowerIters", Flags::Use6PowerIters)
        .value("Exhaustive", Flags::Exhaustive)
        .value("TryAllInitialEndpoints", Flags::TryAllInitialEndpoints)
        .def("__invert__", [](Flags f1) { return ~unsigned(f1); })
        .def("__and__", [](Flags f1, Flags f2) { return unsigned(f1) & unsigned(f2); })
        .def("__rand__", [](Flags f1, Flags f2) { return unsigned(f1) & unsigned(f2); })
        .def("__or__", [](Flags f1, Flags f2) { return unsigned(f1) | unsigned(f2); })
        .def("__ror__", [](Flags f1, Flags f2) { return unsigned(f1) | unsigned(f2); })
        .def("__xor__", [](Flags f1, Flags f2) { return unsigned(f1) ^ unsigned(f2); })
        .def("__rxor__", [](Flags f1, Flags f2) { return unsigned(f2) ^ unsigned(f1); });

    py::enum_<BC1Encoder::EndpointMode>(bc1_encoder, "EndpointMode")
        .value("LeastSquares", BC1Encoder::EndpointMode::LeastSquares)
        .value("BoundingBox", BC1Encoder::EndpointMode::BoundingBox)
        .value("BoundingBoxInt", BC1Encoder::EndpointMode::BoundingBoxInt)
        .value("PCA", BC1Encoder::EndpointMode::PCA);
}

}  // namespace rgbcx::bindings