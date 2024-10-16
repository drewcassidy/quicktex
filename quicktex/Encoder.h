/*  Quicktex Texture Compression Library
    Copyright (C) 2021-2024 Andrew Cassidy <drewcassidy@me.com>
    Partially derived from rgbcx.h written by Richard Geldreich 2020 <richgel99@gmail.com>
    and licenced under the public domain

 */

#pragma once

#include <memory>

#include "ColorBlock.h"
#include "Texture.h"

namespace quicktex {

template <typename T> class Encoder {
   public:
    using Texture = T;

    virtual ~Encoder() = default;
    virtual T Encode(const RawTexture &decoded) const = 0;
};

template <typename T> class BlockEncoder : public Encoder<T> {
   public:
    inline static constexpr int BlockWidth = T::BlockType::Width;
    inline static constexpr int BlockHeight = T::BlockType::Height;

    using Texture = T;
    using EncodedBlock = typename T::BlockType;
    using DecodedBlock = ColorBlock<BlockWidth, BlockHeight>;

    virtual EncodedBlock EncodeBlock(const DecodedBlock &block) const = 0;

    virtual T Encode(const RawTexture &decoded) const override {
        auto encoded = T(decoded.Width(), decoded.Height());

        int blocks_x = encoded.BlocksX();
        int blocks_y = encoded.BlocksY();

        // from experimentation, multithreading this using OpenMP sometimes actually makes encoding slower
        // due to thread creation/teardown taking longer than the encoding process itself.
        // As a result, this is sometimes left as a serial operation despite being embarassingly parallelizable
        // threshold for number of blocks before multithreading is set by overriding MTThreshold()
#pragma omp parallel for if (blocks_x * blocks_y >= MTThreshold())
        for (int y = 0; y < blocks_y; y++) {
            for (int x = 0; x < blocks_x; x++) {
                auto pixels = decoded.GetBlock<BlockWidth, BlockHeight>(x, y);
                auto block = EncodeBlock(pixels);
                encoded.SetBlock(x, y, block);
            }
        }

        return encoded;
    }

    virtual size_t MTThreshold() const { return SIZE_MAX; };
};
}  // namespace quicktex
