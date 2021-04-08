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
#include "BC4Decoder.h"
#include "BC4Encoder.h"

namespace py = pybind11;
namespace quicktex::bindings {

using namespace quicktex::s3tc;

void InitBC4(py::module_ &s3tc) {
    auto bc4 = s3tc.def_submodule("_bc4", "internal bc4 module");

    // region BC4Block
    auto bc4_block = BindBlock<BC4Block>(bc4, "BC4Block");
    bc4_block.doc() = "A single BC4 block.";

    bc4_block.def(py::init<>());
    bc4_block.def(py::init<uint8_t, uint8_t, BC4Block::SelectorArray>(), "endpoint0"_a, "endpoint1"_a, "selectors"_a, R"doc(
        Create a new BC4Block with the specified endpoints and selectors.

        :param int endpoint0: The first endpoint.
        :param int endpoint1: The second endpoint.
        :param selectors: the selectors as a 4x4 list of integers, between 0 and 7 inclusive.
    )doc");

    bc4_block.def_property("endpoints", &BC4Block::GetAlphas, &BC4Block::SetAlphas, "The block's endpoint values as a 2-tuple.");
    bc4_block.def_property("selectors", &BC4Block::GetSelectors, &BC4Block::SetSelectors, R"doc(
        The block's selectors as a 4x4 list of integers between 0 and 7 inclusive.

        .. note::
            This is a property, so directly modifying its value will not propogate back to the block.
            Instead you must read, modify, then write the new list back to the property, like so::

                selectors = block.selectors
                selectors[0,0] = 0
                block.selectors = selectors
    )doc");
    bc4_block.def_property_readonly("values", &BC4Block::GetValues, R"doc(
        The interpolated values used to decode the block, coresponding with the indices in :py:attr:`selectors`.
    )doc");
    bc4_block.def_property_readonly("is_6value", &BC4Block::Is6Value, R"doc(
        "True if the block uses 6-value interpolation, i.e. endpoint0 <= endpoint1. Readonly.
    )doc");
    // endregion

    // region BC4Texture
    auto bc4_texture = BindBlockTexture<BC4Block>(bc4, "BC4Texture");
    bc4_texture.doc() = "A texture comprised of BC4 blocks.";
    // endregion

    // region BC4Encoder
    py::class_<BC4Encoder> bc4_encoder(bc4, "BC4Encoder", R"doc(
        Encodes single-channel textures to BC4.
    )doc");

    bc4_encoder.def(py::init<uint8_t>(), py::arg("channel") = 3, R"doc(
        Create a new BC4 encoder with the specified channel

        :param int channel: the channel that will be read from. 0 to 3 inclusive. Default: 3 (alpha).
    )doc");

    bc4_encoder.def("encode", &BC4Encoder::Encode, "texture"_a, R"doc(
        Encode a raw texture into a new BC4Texture using the encoder's current settings.

        :param RawTexture texture: Input texture to encode.
        :returns: A new BC4Texture with the same dimension as the input.
    )doc");
    
    bc4_encoder.def_property_readonly("channel", &BC4Encoder::GetChannel, "The channel that will be read from. 0 to 3 inclusive. Readonly.");
    // endregion

    // region BC4Decoder
    py::class_<BC4Decoder> bc4_decoder(bc4, "BC4Decoder", R"doc(
        Decodes BC4 textures to a single-channel.
    )doc");

    bc4_decoder.def(py::init<uint8_t>(), py::arg("channel") = 3, R"doc(
        Create a new BC4 decoder with the specified channel

        :param int channel: The channel that will be written to. 0 to 3 inclusive. Default: 3 (alpha).
    )doc");

    bc4_decoder.def("decode", &BC4Decoder::Decode, "texture"_a, R"doc(
        Decode a BC4 texture into a new RawTexture using the decoder's current settings.

        :param RawTexture texture: Input texture to encode.
        :returns: A new RawTexture with the same dimensions as the input
    )doc");
    
    bc4_decoder.def_property_readonly("channel", &BC4Decoder::GetChannel, "The channel that will be written to. 0 to 3 inclusive. Readonly.");
    // endregion
}

}  // namespace quicktex::bindings