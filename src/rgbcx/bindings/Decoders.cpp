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

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <array>

#include "../BC1/BC1Decoder.h"
#include "../BC3/BC3Decoder.h"
#include "../BC4/BC4Decoder.h"
#include "../BC5/BC5Decoder.h"
#include "../BlockDecoder.h"

namespace py = pybind11;
namespace rgbcx::bindings {

py::bytes DecodeImage(const BlockDecoder &self, py::bytes encoded, unsigned image_width, unsigned image_height) {
    if (image_width % self.BlockWidth() != 0) throw std::invalid_argument("Width is not an even multiple of block_width");
    if (image_height % self.BlockHeight() != 0) throw std::invalid_argument("Height is not an even multiple of block_height");
    if (image_width == 0 || image_height == 0) throw std::invalid_argument("Image has zero size");

    size_t size = image_width * image_height;
    size_t block_size = (size / (self.BlockHeight() * self.BlockWidth())) * self.BlockSize();
    size_t color_size = size * sizeof(Color);

    std::string encoded_str = (std::string)encoded;  // encoded data is copied here, unfortunately
    std::string decoded_str = std::string(color_size, 0);

    if (encoded_str.size() != block_size) throw std::invalid_argument("Incompatible data: image width and height do not match the size of the encoded image");

    self.DecodeImage(reinterpret_cast<uint8_t *>(encoded_str.data()), reinterpret_cast<Color *>(decoded_str.data()), image_width, image_height);

    auto bytes = py::bytes(decoded_str);  // decoded data is copied here, unfortunately

    return bytes;
}

void InitDecoders(py::module_ &m) {
    // BlockDecoder
    py::class_<BlockDecoder> block_decoder(m, "BlockDecoder");

    block_decoder.def("decode_image", &DecodeImage);
    block_decoder.def_property_readonly("block_size", &BlockDecoder::BlockSize);
    block_decoder.def_property_readonly("block_width", &BlockDecoder::BlockWidth);
    block_decoder.def_property_readonly("block_height", &BlockDecoder::BlockHeight);

    // BC1Decoder
    py::class_<BC1Decoder> bc1_decoder(m, "BC1Decoder", block_decoder);

    bc1_decoder.def(py::init<Interpolator::Type, bool>(), py::arg("interpolator") = Interpolator::Type::Ideal, py::arg("write_alpha") = false);
    bc1_decoder.def_property_readonly("interpolator_type", &BC1Decoder::GetInterpolatorType);
    bc1_decoder.def_readwrite("write_alpha", &BC1Decoder::write_alpha);

    // BC3Decoder
    py::class_<BC3Decoder> bc3_decoder(m, "BC3Decoder", block_decoder);

    bc3_decoder.def(py::init<Interpolator::Type>(), py::arg("type") = Interpolator::Type::Ideal);
    bc3_decoder.def_property_readonly("bc1_decoder", &BC3Decoder::GetBC1Decoder);
    bc3_decoder.def_property_readonly("bc4_decoder", &BC3Decoder::GetBC4Decoder);

    // BC4Decoder
    py::class_<BC4Decoder> bc4_decoder(m, "BC4Decoder", block_decoder);

    bc4_decoder.def(py::init<uint8_t>(), py::arg("channel") = 3);
    bc4_decoder.def_property("channel", &BC4Decoder::GetChannel, &BC4Decoder::SetChannel);

    // BC5Decoder
    py::class_<BC5Decoder> bc5_decoder(m, "BC5Decoder", block_decoder);

    bc5_decoder.def(py::init<uint8_t, uint8_t>(), py::arg("chan0") = 0, py::arg("chan1") = 1);
    bc5_decoder.def_property("channels", &BC5Decoder::GetChannels, &BC5Decoder::SetChannels);
    bc5_decoder.def_property_readonly("bc4_decoders", &BC5Decoder::GetBC4Decoders);
}

}  // namespace rgbcx::bindings