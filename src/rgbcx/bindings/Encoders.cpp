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

#include "../BC1/BC1Encoder.h"
#include "../BlockEncoder.h"
#include "../Color.h"
#include "../Interpolator.h"

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;
namespace rgbcx::bindings {

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

void InitEncoders(py::module_ &m) {
    // BlockEncoder
    py::class_<BlockEncoder> block_encoder(m, "BlockEncoder");

    block_encoder.def("encode_image", &EncodeImage);
    block_encoder.def_property_readonly("block_size", &BlockEncoder::BlockSize);
    block_encoder.def_property_readonly("block_width", &BlockEncoder::BlockWidth);
    block_encoder.def_property_readonly("block_height", &BlockEncoder::BlockHeight);

    // BC1Encoder
    py::class_<BC1Encoder> bc1_encoder(m, "BC1Encoder", block_encoder);

    bc1_encoder.def(py::init<Interpolator::Type, unsigned, bool, bool>(), py::arg("interpolator") = Interpolator::Type::Ideal, py::arg("level") = 5,
                    py::arg("use_3color") = true, py::arg("use_3color_black") = true);
    bc1_encoder.def("set_level", &BC1Encoder::SetLevel);
    bc1_encoder.def_property_readonly("interpolator_type", &BC1Encoder::GetInterpolatorType);
    bc1_encoder.def_property("flags", &BC1Encoder::GetFlags, &BC1Encoder::SetFlags);
    bc1_encoder.def_property("error_mode", &BC1Encoder::GetErrorMode, &BC1Encoder::SetErrorMode);
    bc1_encoder.def_property("endpoint_mode", &BC1Encoder::GetEndpointMode, &BC1Encoder::SetEndpointMode);
    bc1_encoder.def_property("search_rounds", &BC1Encoder::GetSearchRounds, &BC1Encoder::SetSearchRounds);
    bc1_encoder.def_property("orderings_4", &BC1Encoder::GetOrderings4, &BC1Encoder::SetOrderings4);
    bc1_encoder.def_property("orderings_3", &BC1Encoder::GetOrderings3, &BC1Encoder::SetOrderings3);

    using Flags = BC1Encoder::Flags;
    py::enum_<Flags>(bc1_encoder, "Flags", py::arithmetic())
        .value("UseLikelyTotalOrderings", Flags::UseLikelyTotalOrderings)
        .value("TwoLeastSquaresPasses", Flags::TwoLeastSquaresPasses)
        .value("Use3ColorBlocksForBlackPixels", Flags::Use3ColorBlocksForBlackPixels)
        .value("Use3ColorBlocks", Flags::Use3ColorBlocks)
        .value("Iterative", Flags::Iterative)
        .value("Use6PowerIters", Flags::Use6PowerIters)
        .value("Exhaustive", Flags::Exhaustive)
        .value("TryAllInitialEndpoints", Flags::TryAllInitialEndpoints)
        .def("__invert__", [](Flags f1) { return ~unsigned(f1); })
        .def("__and__", [](Flags f1, Flags f2) { return unsigned(f1) & unsigned(f2); })
        .def("__rand__", [](Flags f1, Flags f2) { return unsigned(f1) & unsigned(f2); })
        .def("__or__", [](Flags f1, Flags f2) { return unsigned(f1) | unsigned(f2); })
        .def("__ror__", [](Flags f1, Flags f2) { return unsigned(f1) | unsigned(f2); })
        .def("__xor__", [](Flags f1, Flags f2) { return unsigned(f1) ^ unsigned(f2); })
        .def("__rxor__", [](Flags f1, Flags f2) { return unsigned(f2) ^ unsigned(f1); });

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
}

}  // namespace rgbcx::bindings