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

#include <array>
#include <cstdint>

#include "../../Decoder.h"
#include "../../Encoder.h"
#include "BC5Decoder.h"
#include "BC5Encoder.h"

namespace py = pybind11;
namespace quicktex::bindings {

using namespace quicktex::s3tc;
using namespace quicktex::s3tc;

void InitBC5(py::module_ &s3tc) {
    auto bc5 = s3tc.def_submodule("_bc5", "internal bc5 module");

    // region BC5Block
    auto bc5_block = BindBlock<BC5Block>(bc5, "BC5Block");
    bc5_block.doc() = "A single BC5 block.";

    bc5_block.def(py::init<>());
    bc5_block.def(py::init<BC4Block, BC4Block>(), "chan0_block"_a, "chan1_block"_a, R"doc(
        Create a new BC5Block out of two BC4 blocks.

        :param BC4Block chan0_block: The BC4 block used for the first channel.
        :param BC4Block chan1_block: The BC1 block used for the second channel.
    )doc");

    bc5_block.def_readwrite("chan0_block", &BC5Block::chan0_block, "The BC4 block used for the first channel.");
    bc5_block.def_readwrite("chan1_block", &BC5Block::chan1_block, "The BC4 block used for the second channel.");
    bc5_block.def_property("blocks", &BC5Block::GetBlocks, &BC5Block::SetBlocks, "The BC4 and BC1 blocks that make up this block as a 2-tuple.");
    // endregion

    // region BC5Texture
    auto bc5_texture = BindBlockTexture<BC5Block>(bc5, "BC5Texture");
    bc5_texture.doc() = "A texture comprised of BC5 blocks.";
    // endregion

    // region BC5Encoder
    py::class_<BC5Encoder> bc5_encoder(bc5, "BC5Encoder", R"doc(
        Encodes dual-channel textures to BC5.
    )doc");

    bc5_encoder.def(py::init<uint8_t, uint8_t>(), py::arg("chan0") = 0, py::arg("chan1") = 1, R"doc(
        Create a new BC5 encoder with the specified channels

        :param int chan0: the first channel that will be read from. 0 to 3 inclusive. Default: 0 (red).
        :param int chan1: the second channel that will be read from. 0 to 3 inclusive. Default: 1 (green).
    )doc");

    bc5_encoder.def("encode", &BC5Encoder::Encode, "texture"_a, R"doc(
        Encode a raw texture into a new BC5Texture using the encoder's current settings.

        :param RawTexture texture: Input texture to encode.
        :returns: A new BC5Texture with the same dimension as the input.
    )doc");

    bc5_encoder.def_property_readonly("channels", &BC5Encoder::GetChannels, "A 2-tuple of channels that will be read from. 0 to 3 inclusive. Readonly.");
    bc5_encoder.def_property_readonly("bc4_encoders", &BC5Encoder::GetBC4Encoders,
                                      "2-tuple of internal :py:class:`~quicktex.s3tc.bc4.BC4Encoder` s used for each channel. Readonly.");
    // endregion

    // region BC5Decoder
    py::class_<BC5Decoder> bc5_decoder(bc5, "BC5Decoder", R"doc(
        Decodes BC4 textures to two channels.
    )doc");

    bc5_decoder.def(py::init<uint8_t, uint8_t>(), py::arg("chan0") = 0, py::arg("chan1") = 1, R"doc(
        Create a new BC5 decoder with the specified channels

        :param int chan0: the first channel that will be written to. 0 to 3 inclusive. Default: 0 (red).
        :param int chan1: the second channel that will be written to. 0 to 3 inclusive. Default: 1 (green).
    )doc");

    bc5_decoder.def("decode", &BC5Decoder::Decode, "texture"_a, R"doc(
        Decode a BC5 texture into a new RawTexture using the decoder's current settings.

        :param RawTexture texture: Input texture to encode.
        :returns: A new RawTexture with the same dimensions as the input
    )doc");

    bc5_decoder.def_property_readonly("channels", &BC5Decoder::GetChannels, "A 2-tuple of channels that will be written to. 0 to 3 inclusive. Readonly.");
    bc5_decoder.def_property_readonly("bc4_decoders", &BC5Decoder::GetBC4Decoders,
                                      "2-tuple of internal :py:class:`~quicktex.s3tc.bc4.BC4Decoder` s used for each channel. Readonly.");
    // endregion
}
}  // namespace quicktex::bindings