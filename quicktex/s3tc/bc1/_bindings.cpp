/*  Quicktex Texture Compression Library
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

#include "../../_bindings.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>

#include "../../Decoder.h"
#include "../../Encoder.h"
#include "../interpolator/Interpolator.h"
#include "BC1Decoder.h"
#include "BC1Encoder.h"

namespace py = pybind11;
namespace quicktex::bindings {

using namespace quicktex::s3tc;
using namespace pybind11::literals;

using InterpolatorPtr = std::shared_ptr<Interpolator>;

void InitBC1(py::module_ &s3tc) {
    auto bc1 = s3tc.def_submodule("_bc1", "internal bc1 module");

    // region BC1Block
    auto bc1_block = BindBlock<BC1Block>(bc1, "BC1Block");
    bc1_block.doc() = "A single BC1 block.";

    bc1_block.def(py::init<>());
    bc1_block.def(py::init<Color, Color, BC1Block::SelectorArray>(), "color0"_a, "color1"_a, "selectors"_a, R"doc(
        Create a new BC1Block with the specified endpoints and selectors

        :param color0: The first endpoint
        :param color1: The second endpoint
        :param selectors: the selectors as a 4x4 list of integers, between 0 and 3 inclusive.
    )doc");

    bc1_block.def_property("endpoints", &BC1Block::GetColors, &BC1Block::SetColors, "The block's endpoint colors as a 2-tuple.");
    bc1_block.def_property("selectors", &BC1Block::GetSelectors, &BC1Block::SetSelectors, R"doc(
        The block's selectors as a 4x4 list of integers between 0 and 3 inclusive.

        .. note::
            This is a property, so directly modifying its value will not propogate back to the block.
            Instead you must read, modify, then write the new list back to the property, like so::

                selectors = block.selectors
                selectors[0,0] = 0
                block.selectors = selectors
    )doc");
    bc1_block.def_property_readonly("is_3color", &BC1Block::Is3Color, R"doc(
        "True if the block uses 3-color interpolation, i.e. color0 <= color1. This value should be ignored when decoding as part of a BC3 block. Readonly.
    )doc");
    // endregion

    // region BC1Texture
    auto bc1_texture = BindBlockTexture<BC1Block>(bc1, "BC1Texture");
    bc1_texture.doc() = "A texture comprised of BC1 blocks.";
    // endregion

    // region BC1Encoder
    py::class_<BC1Encoder> bc1_encoder(bc1, "BC1Encoder", "Encodes RGB textures to BC1.");

    py::enum_<BC1Encoder::EndpointMode>(bc1_encoder, "EndpointMode", "Enum representing various methods of finding endpoints in a block.")
        .value("LeastSquares", BC1Encoder::EndpointMode::LeastSquares, "Find endpoints using a 2D least squares approach.")
        .value("BoundingBox", BC1Encoder::EndpointMode::BoundingBox, "Find endpoints using a simple bounding box. Fast but inaccurate.")
        .value("BoundingBoxInt", BC1Encoder::EndpointMode::BoundingBoxInt, "Same as BoundingBox but using integers, slightly faster.")
        .value("PCA", BC1Encoder::EndpointMode::PCA, "Find endpoints using Principle Component Analysis. Slowest but highest quality method.");

    py::enum_<BC1Encoder::ErrorMode>(bc1_encoder, "ErrorMode", "Enum representing various methods of finding selectors in a block.")
        .value("None", BC1Encoder::ErrorMode::None, "The same as Faster but error is not calculated. This disables any cluster-fit options")
        .value("Faster", BC1Encoder::ErrorMode::Faster, "Use a slightly lower quality, but ~30% faster MSE evaluation function for 4-color blocks.")
        .value("Check2", BC1Encoder::ErrorMode::Check2, "Default error-checking method.")
        .value("Full", BC1Encoder::ErrorMode::Full, "Examine all colors to compute selectors/MSE. Slower but slightly higher quality.");

    py::enum_<BC1Encoder::ColorMode>(bc1_encoder, "ColorMode", "Enum representing various methods of writing BC1 blocks.")
        .value("FourColor", BC1Encoder::ColorMode::FourColor, "Default color mode. Only 4-color blocks will be output, where color0 > color1")
        .value("ThreeColor", BC1Encoder::ColorMode::ThreeColor, "Additionally use 3-color blocks when they have a lower error, where color0 <= color1")
        .value("ThreeColorBlack", BC1Encoder::ColorMode::ThreeColorBlack,
               "Additionally use 3-color blocks with black pixels (selector 3). Note that this requires your shader/engine to not sample the alpha channel "
               "when using a BC1 texture.");

    bc1_encoder.def(py::init<unsigned, BC1Encoder::ColorMode>(), "level"_a = 5, "color_mode"_a = BC1Encoder::ColorMode::FourColor);
    bc1_encoder.def(py::init<unsigned, BC1Encoder::ColorMode, InterpolatorPtr>(), "level"_a, "color_mode"_a, "interpolator"_a, R"doc(
        Create a new BC1 encoder with the specified preset level, color mode, and interpolator.

        :param int level: The preset level of the resulting encoder, between 0 and 18 inclusive. See :py:meth:`set_level` for more information. Default: 5.
        :param ColorMode color_mode: The color mode of the resulting BC1Encoder. Default: :py:class:`~quicktex.s3tc.bc1.BC1Encoder.ColorMode.FourColor`.
        :param Interpolator interpolator: The interpolation mode to use for encoding. Default: :py:class:`~quicktex.s3tc.interpolator.Interpolator`.
    )doc");

    bc1_encoder.def("encode", &BC1Encoder::Encode, "texture"_a, R"doc(
        Encode a raw texture into a new BC1Texture using the encoder's current settings.

        :param RawTexture texture: Input texture to encode.
        :returns: A new BC1Texture with the same dimension as the input.
    )doc");

    bc1_encoder.def("set_level", &BC1Encoder::SetLevel, "level"_a, R"doc(
        Select a preset quality level, between 0 and 18 inclusive.  Higher quality levels are slower, but produce blocks that are a closer match to input.
        This has no effect on the size of the resulting texture, since BC1 is a fixed-ratio compression method. For better control, see the advanced API below

        :param int level: The preset level of the resulting encoder, between 0 and 18 inclusive. Default: 5.
    )doc");

    bc1_encoder.def_property_readonly("interpolator", &BC1Encoder::GetInterpolator,
                                      "The :py:class:`~quicktex.s3tc.interpolator.Interpolator` used by this encoder. This is a readonly property.");
    bc1_encoder.def_property_readonly("color_mode", &BC1Encoder::GetColorMode,
                                      "The :py:class:`~quicktex.s3tc.bc1.BC1Encoder.ColorMode` used by this encoder. This is a readonly property.");

    // Advanced API

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

    bc1_encoder.def_readonly_static("max_power_iterations", &BC1Encoder::max_power_iterations);
    bc1_encoder.def_readonly_static("min_power_iterations", &BC1Encoder::min_power_iterations);

    bc1_encoder.def_property("power_iterations", &BC1Encoder::GetPowerIterations, &BC1Encoder::SetPowerIterations,
                             "Number of power iterations used with the PCA endpoint mode. Value should be around 4 to 6. "
                             "Automatically clamped to between :py:const:`BC1Encoder.min_power_iterations` and :py:const:`BC1Encoder.max_power_iterations`");
    // endregion

    // region BC1Decoder
    py::class_<BC1Decoder> bc1_decoder(bc1, "BC1Decoder", R"doc(
        Decodes BC1 textures to RGB
    )doc");

    bc1_decoder.def(py::init<bool>(), "write_alpha"_a = false);
    bc1_decoder.def(py::init<bool, InterpolatorPtr>(), "write_alpha"_a, "interpolator"_a, R"doc(
        Create a new BC1 decoder with the specificied interpolator.

        :param bool write_alpha: Determines if the alpha channel of the output is written to. Default: False;
        :param Interpolator interpolator: The interpolation mode to use for decoding. Default: :py:class:`~quicktex.s3tc.interpolator.Interpolator`.
    )doc");

    bc1_decoder.def("decode", &BC1Decoder::Decode, "texture"_a, R"doc(
        Decode a BC1 texture into a new RawTexture using the decoder's current settings.

        :param RawTexture texture: Input texture to encode.
        :returns: A new RawTexture with the same dimensions as the input
    )doc");

    bc1_decoder.def_property_readonly("interpolator", &BC1Decoder::GetInterpolator, "The interpolator used by this decoder. This is a readonly property.");
    bc1_decoder.def_readwrite("write_alpha", &BC1Decoder::write_alpha, "Determines if the alpha channel of the output is written to.");
    // endregion
}
}  // namespace quicktex::bindings