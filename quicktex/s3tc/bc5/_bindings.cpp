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
#include <cstdint>

#include "../../BlockDecoder.h"
#include "../../BlockEncoder.h"
#include "BC5Decoder.h"
#include "BC5Encoder.h"

namespace py = pybind11;
namespace quicktex::bindings {

using namespace quicktex::s3tc;
using namespace quicktex::s3tc;

void InitBC5(py::module_ &s3tc) {
    auto bc5 = s3tc.def_submodule("_bc5", "BC5 encoding/decoding module");
    auto block_encoder = py::type::of<BlockEncoder>();
    auto block_decoder = py::type::of<BlockDecoder>();

    // BC5Encoder
    py::class_<BC5Encoder> bc5_encoder(bc5, "BC5Encoder", block_encoder);

    bc5_encoder.def(py::init<uint8_t, uint8_t>(), py::arg("chan0") = 0, py::arg("chan1") = 1);
    bc5_encoder.def_property_readonly("channels", &BC5Encoder::GetChannels);
    bc5_encoder.def_property_readonly("bc4_decoders", &BC5Encoder::GetBC4Encoders);

    // BC5Decoder
    py::class_<BC5Decoder> bc5_decoder(bc5, "BC5Decoder", block_decoder);

    bc5_decoder.def(py::init<uint8_t, uint8_t>(), py::arg("chan0") = 0, py::arg("chan1") = 1);
    bc5_decoder.def_property_readonly("channels", &BC5Decoder::GetChannels);
    bc5_decoder.def_property_readonly("bc4_decoders", &BC5Decoder::GetBC4Decoders);
}
}  // namespace quicktex::bindings