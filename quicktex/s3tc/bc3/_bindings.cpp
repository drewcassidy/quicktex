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
#include "BC3Decoder.h"
#include "BC3Encoder.h"

namespace py = pybind11;
namespace quicktex::bindings {

using namespace quicktex::s3tc;
using namespace pybind11::literals;
using InterpolatorPtr = std::shared_ptr<Interpolator>;
using BC1EncoderPtr = std::shared_ptr<BC1Encoder>;
using BC1DecoderPtr = std::shared_ptr<BC1Decoder>;

void InitBC3(py::module_ &s3tc) {
    auto bc3 = s3tc.def_submodule("_bc3", "internal bc3 module");
    auto block_encoder = py::type::of<BlockEncoder>();
    auto block_decoder = py::type::of<BlockDecoder>();
    py::options options;
    options.disable_function_signatures();

    // BC3Encoder
    py::class_<BC3Encoder> bc3_encoder(bc3, "BC3Encoder", block_encoder,R"doc(
        Base: :py:class:`~quicktex.BlockEncoder`

        Encodes RGBA textures to BC3
    )doc");

    bc3_encoder.def(py::init<unsigned>(), "level"_a = 5);
    bc3_encoder.def(py::init<unsigned, InterpolatorPtr>(), "level"_a, "interpolator"_a, R"doc(
        __init__(self, level: int = 5, interpolator=Interpolator()) -> None

        Create a new BC3 encoder with the specified preset level and interpolator.

        :param int level: The preset level of the resulting encoder, between 0 and 18 inclusive.
            See :py:meth:`~quicktex.s3tc.bc1.BC1Encoder.set_level` for more information. Default: 5.
        :param Interpolator interpolator: The interpolation mode to use for encoding. Default: :py:class:`~quicktex.s3tc.interpolator.Interpolator`.
    )doc");

    bc3_encoder.def_property_readonly("bc1_encoder", &BC3Encoder::GetBC1Encoder,
                                      "Internal :py:class:`~quicktex.s3tc.bc1.BC1Encoder` used for RGB data. Readonly.");
    bc3_encoder.def_property_readonly("bc4_encoder", &BC3Encoder::GetBC4Encoder,
                                      "Internal :py:class:`~quicktex.s3tc.bc4.BC4Encoder` used for alpha data. Readonly.");

    // BC3Decoder
    py::class_<BC3Decoder> bc3_decoder(bc3, "BC3Decoder", block_decoder, R"doc(
        Base: :py:class:`~quicktex.BlockDecoder`

        Decodes BC3 textures to RGBA
    )doc");

    bc3_decoder.def(py::init<>());
    bc3_decoder.def(py::init<InterpolatorPtr>(), "interpolator"_a, R"doc(
        __init__(interpolator = Interpolator()) -> None

        Create a new BC3 decoder with the specified interpolator.

        :param Interpolator interpolator: The interpolation mode to use for decoding. Default: :py:class:`~quicktex.s3tc.interpolator.Interpolator`.
    )doc");

    bc3_decoder.def_property_readonly("bc1_decoder", &BC3Decoder::GetBC1Decoder,
                                      "Internal :py:class:`~quicktex.s3tc.bc1.BC1Decoder` used for RGB data. Readonly.");
    bc3_decoder.def_property_readonly("bc4_decoder", &BC3Decoder::GetBC4Decoder,
                                      "Internal :py:class:`~quicktex.s3tc.bc4.BC4Decoder` used for alpha data. Readonly.");
};
}  // namespace quicktex::bindings