/*  Python-rgbcx Texture Compression Library
    Copyright (C) 2021 Andrew Cassidy <drewcassidy@me.com>
    Partially derived from rgbcx.h written by Richard Geldreich 2020 <richgel99@gmail.com>
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

#pragma once

#include <cmath>
#include <cstdint>
#include <span>
#include <vector>

#include "ColorBlock.h"
#include "util.h"

namespace rgbcx {

template <class B, size_t M, size_t N> class BlockDecoder {
   public:
    using DecodedBlock = ColorBlock<M, N>;
    using EncodedBlock = B;

    BlockDecoder() noexcept = default;
    virtual ~BlockDecoder() noexcept = default;
    virtual void DecodeBlock(DecodedBlock dest, EncodedBlock *const block) const = 0;

    void DecodeRow(std::span<DecodedBlock> dests, std::span<const EncodedBlock> blocks) {
        assert(dests.size() == blocks.size());

        for (int i = 0; i < dests.size; i++) { DecodeBlock(&dests[i], &blocks[i]); }
    }

    std::vector<Color> DecodeImage(uint8_t *bytes, unsigned image_width, unsigned image_height, unsigned chunk_size = 0, bool threaded = false) {
        assert(threaded == chunk_size > 0);
        unsigned block_width = maximum(1U, ((image_width + 3) / 4));
        unsigned block_height = maximum(1U, ((image_height + 3) / 4));
        using Row = typename DecodedBlock::Row;

        auto image = std::vector<Color>(block_width * block_height * N * M);
        auto blocks = reinterpret_cast<B *>(bytes);

        if (!threaded) {
            for (unsigned x = 0; x < block_width; x++) {
                for (unsigned y = 0; y < block_height; y++) {
                    unsigned pixel_x = x * N;
                    unsigned pixel_y = y * M;

                    assert(pixel_x >= 0);
                    assert(pixel_y >= 0);
                    assert(pixel_y + M <= image_height);
                    assert(pixel_x + N <= image_width);

                    unsigned top_left = pixel_x + (pixel_y * image_width);
                    auto rows = std::array<Row *, M>();
                    for (unsigned i = 0; i < M; i++) { rows[i] = reinterpret_cast<Row *>(&image[top_left + i * image_width]); }

                    // auto dest = DecodedBlock(image, image_width, image_height, x, y);
                    DecodeBlock(DecodedBlock(rows), &blocks[x + block_width * y]);
                }
            }
        }

        return image;
    }
};
}  // namespace rgbcx
