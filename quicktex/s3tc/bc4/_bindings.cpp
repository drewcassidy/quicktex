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
#include "BC4Decoder.h"
#include "BC4Encoder.h"

namespace py = pybind11;
namespace quicktex::bindings {

using namespace quicktex::s3tc;
using namespace quicktex::s3tc;

void InitBC4(py::module_ &s3tc) {
    auto bc4 = s3tc.def_submodule("_bc4", "internal bc4 module");
    auto block_encoder = py::type::of<BlockEncoder>();
    auto block_decoder = py::type::of<BlockDecoder>();
    py::options options;
    options.disable_function_signatures();

    // BC4Encoder
    py::class_<BC4Encoder> bc4_encoder(bc4, "BC4Encoder", block_encoder, "Encodes single-channel textures to BC4.");

    bc4_encoder.def(py::init<uint8_t>(), py::arg("channel") = 3, R"doc(
        __init__(channel : int = 3) -> None

        Create a new BC4 encoder with the specified channel

        :param int channel: the channel that will be read from. 0 to 3 inclusive. Default: 3 (alpha).
    )doc");
    bc4_encoder.def_property_readonly("channel", &BC4Encoder::GetChannel, "The channel that will be read from. 0 to 3 inclusive. Readonly.");

    // BC4Decoder
    py::class_<BC4Decoder> bc4_decoder(bc4, "BC4Decoder", block_decoder, "Encodes BC4 textures to a single-channel texture.");

    bc4_decoder.def(py::init<uint8_t>(), py::arg("channel") = 3, R"doc(
        __init__(channel : int = 3) -> None

        Create a new BC4 decoder with the specified channel

        :param int channel: The channel that will be written to. 0 to 3 inclusive. Default: 3 (alpha).
    )doc");
    bc4_decoder.def_property_readonly("channel", &BC4Decoder::GetChannel, "The channel that will be written to. 0 to 3 inclusive. Readonly.");
}

}  // namespace quicktex::bindings