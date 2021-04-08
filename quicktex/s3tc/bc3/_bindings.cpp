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
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>

#include "../../Decoder.h"
#include "../../Encoder.h"
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

    // region BC3Block
    auto bc3_block = BindBlock<BC3Block>(bc3, "BC3Block");
    bc3_block.doc() = "A single BC3 block.";

    bc3_block.def(py::init<>());
    bc3_block.def(py::init<BC4Block, BC1Block>(), "alpha_block"_a, "color_block"_a, R"doc(
        Create a new BC3Block out of a BC4 block and a BC1 block.

        :param BC4Block alpha_block: The BC4 block used for alpha data.
        :param BC1Block color_block: The BC1 block used for RGB data.
    )doc");

    bc3_block.def_readwrite("alpha_block", &BC3Block::alpha_block, "The BC4 block used for alpha data.");
    bc3_block.def_readwrite("color_block", &BC3Block::color_block, "The BC1 block used for rgb data.");
    bc3_block.def_property("blocks", &BC3Block::GetBlocks, &BC3Block::SetBlocks, "The BC4 and BC1 blocks that make up this block as a 2-tuple.");
    // endregion

    // region BC3Texture
    auto bc3_texture = BindBlockTexture<BC3Block>(bc3, "BC3Texture");
    bc3_texture.doc() = "A texture comprised of BC3 blocks.";
    // endregion

    // region BC3Encoder
    py::class_<BC3Encoder> bc3_encoder(bc3, "BC3Encoder", R"doc(
        Encodes RGBA textures to BC3
    )doc");

    bc3_encoder.def(py::init<unsigned>(), "level"_a = 5);
    bc3_encoder.def(py::init<unsigned, InterpolatorPtr>(), "level"_a, "interpolator"_a, R"doc(
        Create a new BC3 encoder with the specified preset level and interpolator.

        :param int level: The preset level of the resulting encoder, between 0 and 18 inclusive.
            See :py:meth:`~quicktex.s3tc.bc1.BC1Encoder.set_level` for more information. Default: 5.
        :param Interpolator interpolator: The interpolation mode to use for encoding. Default: :py:class:`~quicktex.s3tc.interpolator.Interpolator`.
    )doc");

    bc3_encoder.def("encode", &BC3Encoder::Encode, "texture"_a, R"doc(
        Encode a raw texture into a new BC3Texture using the encoder's current settings.

        :param RawTexture texture: Input texture to encode.
        :returns: A new BC3Texture with the same dimension as the input.
    )doc");

    bc3_encoder.def_property_readonly("bc1_encoder", &BC3Encoder::GetBC1Encoder,
                                      "Internal :py:class:`~quicktex.s3tc.bc1.BC1Encoder` used for RGB data. Readonly.");
    bc3_encoder.def_property_readonly("bc4_encoder", &BC3Encoder::GetBC4Encoder,
                                      "Internal :py:class:`~quicktex.s3tc.bc4.BC4Encoder` used for alpha data. Readonly.");
    // endregion

    // region BC3Decoder
    py::class_<BC3Decoder> bc3_decoder(bc3, "BC3Decoder", R"doc(
        Decodes BC3 textures to RGBA
    )doc");

    bc3_decoder.def(py::init<>());
    bc3_decoder.def(py::init<InterpolatorPtr>(), "interpolator"_a, R"doc(
        Create a new BC3 decoder with the specified interpolator.

        :param Interpolator interpolator: The interpolation mode to use for decoding. Default: :py:class:`~quicktex.s3tc.interpolator.Interpolator`.
    )doc");

    bc3_decoder.def("decode", &BC3Decoder::Decode, "texture"_a, R"doc(
        Decode a BC3 texture into a new RawTexture using the decoder's current settings.

        :param RawTexture texture: Input texture to encode.
        :returns: A new RawTexture with the same dimensions as the input
    )doc");

    bc3_decoder.def_property_readonly("bc1_decoder", &BC3Decoder::GetBC1Decoder,
                                      "Internal :py:class:`~quicktex.s3tc.bc1.BC1Decoder` used for RGB data. Readonly.");
    bc3_decoder.def_property_readonly("bc4_decoder", &BC3Decoder::GetBC4Decoder,
                                      "Internal :py:class:`~quicktex.s3tc.bc4.BC4Decoder` used for alpha data. Readonly.");
    // endregion
}
}  // namespace quicktex::bindings