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

template <class T> class Decoder {
   public:
    using Texture = T;

    virtual ~Decoder() = default;
    virtual RawTexture Decode(const T &encoded) const = 0;
};

template <class T> class BlockDecoder : public Decoder<T> {
   public:
    inline static constexpr int BlockWidth = T::BlockType::Width;
    inline static constexpr int BlockHeight = T::BlockType::Height;

    using Texture = T;
    using EncodedBlock = typename T::BlockType;
    using DecodedBlock = ColorBlock<BlockWidth, BlockHeight>;

    virtual DecodedBlock DecodeBlock(const EncodedBlock &block) const = 0;

    virtual RawTexture Decode(const T &encoded) const override {
        auto decoded = RawTexture(encoded.Width(), encoded.Height());

        int blocks_x = encoded.BlocksX();
        int blocks_y = encoded.BlocksY();

        // from experimentation, multithreading this using OpenMP actually makes decoding slower
        // due to thread creation/teardown taking longer than the decoding process itself.
        // As a result, this is left as a serial operation despite being embarassingly parallelizable
        for (int y = 0; y < blocks_y; y++) {
            for (int x = 0; x < blocks_x; x++) {
                auto block = encoded.GetBlock(x, y);
                auto pixels = DecodeBlock(block);
                decoded.SetBlock<BlockWidth, BlockHeight>(x, y, pixels);
            }
        }

        return decoded;
    }
};
}  // namespace quicktex
