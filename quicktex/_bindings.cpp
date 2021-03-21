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

#include "BlockDecoder.h"
#include "BlockEncoder.h"

namespace py = pybind11;
namespace quicktex::bindings {

void InitS3TC(py::module_ &m);

py::bytes EncodeImage(const BlockEncoder &self, py::bytes decoded, unsigned image_width, unsigned image_height) {
    if (image_width % self.BlockWidth() != 0) throw std::invalid_argument("Width is not an even multiple of block_width");
    if (image_height % self.BlockHeight() != 0) throw std::invalid_argument("Height is not an even multiple of block_height");
    if (image_width == 0 || image_height == 0) throw std::invalid_argument("Image has zero size");

    size_t size = image_width * image_height;
    size_t block_size = (size / (self.BlockHeight() * self.BlockWidth())) * self.BlockSize();
    size_t color_size = size * sizeof(Color);

    std::string encoded_str = std::string(block_size, 0);
    std::string decoded_str = (std::string)decoded;  // decoded data is copied here, unfortunately

    if (decoded_str.size() != color_size) throw std::invalid_argument("Incompatible data: image width and height do not match the size of the decoded image");

    self.EncodeImage(reinterpret_cast<uint8_t *>(encoded_str.data()), reinterpret_cast<Color *>(decoded_str.data()), image_width, image_height);

    auto bytes = py::bytes(encoded_str);  // encoded data is copied here, unfortunately

    return bytes;
}

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

PYBIND11_MODULE(_quicktex, m) {
    m.doc() = "More Stuff";

    py::options options;

    // BlockDecoder
    py::class_<BlockDecoder> block_decoder(m, "BlockDecoder");

    block_decoder.def("decode_image", &DecodeImage);
    block_decoder.def_property_readonly("block_size", &BlockDecoder::BlockSize);
    block_decoder.def_property_readonly("block_width", &BlockDecoder::BlockWidth);
    block_decoder.def_property_readonly("block_height", &BlockDecoder::BlockHeight);

    // BlockEncoder
    py::class_<BlockEncoder> block_encoder(m, "BlockEncoder");

    block_encoder.def("encode_image", &EncodeImage);
    block_encoder.def_property_readonly("block_size", &BlockEncoder::BlockSize);
    block_encoder.def_property_readonly("block_width", &BlockEncoder::BlockWidth);
    block_encoder.def_property_readonly("block_height", &BlockEncoder::BlockHeight);

    InitS3TC(m);
}

}  // namespace quicktex::bindings