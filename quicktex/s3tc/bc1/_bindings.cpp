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
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>

#include "../../BlockDecoder.h"
#include "../../BlockEncoder.h"
#include "../interpolator/Interpolator.h"
#include "BC1Decoder.h"
#include "BC1Encoder.h"

namespace py = pybind11;
namespace quicktex::bindings {

using namespace quicktex::s3tc;
using namespace pybind11::literals;

using InterpolatorPtr = std::shared_ptr<Interpolator>;

void InitBC1(py::module_ &s3tc) {
    auto bc1 = s3tc.def_submodule("_bc1", "BC1 encoding/decoding module");
    auto block_encoder = py::type::of<BlockEncoder>();
    auto block_decoder = py::type::of<BlockDecoder>();

    // BC1Encoder
    py::class_<BC1Encoder> bc1_encoder(bc1, "BC1Encoder", block_encoder, "Encodes RGB textures to BC1");

    py::enum_<BC1Encoder::EndpointMode>(bc1_encoder, "EndpointMode")
        .value("LeastSquares", BC1Encoder::EndpointMode::LeastSquares)
        .value("BoundingBox", BC1Encoder::EndpointMode::BoundingBox)
        .value("BoundingBoxInt", BC1Encoder::EndpointMode::BoundingBoxInt)
        .value("PCA", BC1Encoder::EndpointMode::PCA);

    py::enum_<BC1Encoder::ErrorMode>(bc1_encoder, "ErrorMode")
        .value("None", BC1Encoder::ErrorMode::None)
        .value("Faster", BC1Encoder::ErrorMode::Faster)
        .value("Check2", BC1Encoder::ErrorMode::Check2)
        .value("Full", BC1Encoder::ErrorMode::Full);

    py::enum_<BC1Encoder::ColorMode>(bc1_encoder, "ColorMode")
        .value("FourColor", BC1Encoder::ColorMode::FourColor, "Default color mode. Only 4-color blocks will be output, where color0 > color1")
        .value("ThreeColor", BC1Encoder::ColorMode::ThreeColor)
        .value("ThreeColorBlack", BC1Encoder::ColorMode::ThreeColorBlack);

    bc1_encoder.def(py::init<unsigned, BC1Encoder::ColorMode>(), "level"_a = 5, "color_mode"_a = BC1Encoder::ColorMode::FourColor);
    bc1_encoder.def(py::init<unsigned, BC1Encoder::ColorMode, InterpolatorPtr>(), "level"_a, "color_mode"_a, "interpolator"_a);

    bc1_encoder.def("set_level", &BC1Encoder::SetLevel, "Use a preset quality level, between 0 and 18. For better control, see the advanced API below");

    // Advanced API

    bc1_encoder.def_readonly_static("max_power_iterations", &BC1Encoder::max_power_iterations, "Maximum value of :py:attr:`BC1Encoder.power_iterations`.");
    bc1_encoder.def_readonly_static("min_power_iterations", &BC1Encoder::min_power_iterations, "Minimum value of :py:attr:`BC1Encoder.power_iterations`.");

    bc1_encoder.def_property_readonly("interpolator", &BC1Encoder::GetInterpolator, "The interpolator used by this encoder. This is a readonly property.");
    bc1_encoder.def_property_readonly("color_mode", &BC1Encoder::GetColorMode, "The color mode used by this encoder. This is a readonly property.");

    bc1_encoder.def_property("error_mode", &BC1Encoder::GetErrorMode, &BC1Encoder::SetErrorMode, "The error mode used by this encoder for finding selectors.");
    bc1_encoder.def_property("endpoint_mode", &BC1Encoder::GetEndpointMode, &BC1Encoder::SetEndpointMode, "The endpoint mode used by this encoder.");

    bc1_encoder.def_readwrite("two_ls_passes", &BC1Encoder::two_ls_passes,
                              "Use 2 least squares pass, instead of one (same as stb_dxt's HIGHQUAL option).\n"
                              "Recommended if you're setting the orderings settings greater than 0.");

    bc1_encoder.def_readwrite("two_ep_passes", &BC1Encoder::two_ep_passes, "Try 2 different ways of choosing the initial endpoints.");

    bc1_encoder.def_readwrite("two_cf_passes", &BC1Encoder::two_cf_passes,
                              "Greatly increase encode time, with very slightly higher quality.\n"
                              "Same as squish's iterative cluster fit option. Not really worth the tiny boost in quality, "
                              "unless you just don't care about performance at all.");

    bc1_encoder.def_readwrite("exhaustive", &BC1Encoder::exhaustive,
                              "Check all total orderings - *very* slow. The encoder is not designed to be used in this way");

    bc1_encoder.def_property("search_rounds", &BC1Encoder::GetSearchRounds, &BC1Encoder::SetSearchRounds,
                             "Setting search rounds > 0 enables refining the final endpoints by examining nearby colors. A higher value has a higher quality "
                             "at the expense of performance.");

    bc1_encoder.def_property("orderings", &BC1Encoder::GetOrderings, &BC1Encoder::SetOrderings,
                             "setting the orderings > 0 enables ordered cluster fit using a lookup table of similar blocks. Value is a tuple of (4 color "
                             "orders, 3 color orders), where higher values have a higher quality at the expense of performance.");

    bc1_encoder.def_property("power_iterations", &BC1Encoder::GetPowerIterations, &BC1Encoder::SetPowerIterations,
                             "Number of power iterations used with the PCA endpoint mode. Value should be around 4 to 6. "
                             "Automatically clamped to between :py:const:`BC1Encoder.min_power_iterations` and :py:const:`BC1Encoder.max_power_iterations`");

    // BC1Decoder
    py::class_<BC1Decoder> bc1_decoder(bc1, "BC1Decoder", block_decoder, "Decodes BC1 textures to RGB");

    bc1_decoder.def(py::init<bool>(), "write_alpha"_a = false);
    bc1_decoder.def(py::init<bool, InterpolatorPtr>(), "write_alpha"_a, "interpolator"_a);

    bc1_decoder.def_property_readonly("interpolator", &BC1Decoder::GetInterpolator);
    bc1_decoder.def_readwrite("write_alpha", &BC1Decoder::write_alpha);
}
}  // namespace quicktex::bindings