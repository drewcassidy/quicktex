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

#pragma once

#include <array>
#include <climits>
#include <cstdint>
#include <memory>

#include "../BlockEncoder.h"
#include "../BlockView.h"
#include "../Color.h"
#include "BC1Block.h"
#include "SingleColorTable.h"

namespace rgbcx {
class Interpolator;
class Vector4;

class BC1Encoder final : public BlockEncoder<BC1Block, 4, 4> {
   public:
    using InterpolatorPtr = std::shared_ptr<Interpolator>;

    enum class Flags : uint32_t {
        None = 0,

        // Try to improve quality using the most likely total orderings.
        // The total_orderings_to_try parameter will then control the number of total orderings to try for 4 color blocks, and the
        // total_orderings_to_try3 parameter will control the number of total orderings to try for 3 color blocks (if they are enabled).
        UseLikelyTotalOrderings = 2,

        // Use 2 least squares pass, instead of one (same as stb_dxt's HIGHQUAL option).
        // Recommended if you're enabling UseLikelyTotalOrderings.
        TwoLeastSquaresPasses = 4,

        // Use3ColorBlocksForBlackPixels allows the BC1 encoder to use 3-color blocks for blocks containing black or very dark pixels.
        // You shader/engine MUST ignore the alpha channel on textures encoded with this flag.
        // Average quality goes up substantially for my 100 texture corpus (~.5 dB), so it's worth using if you can.
        // Note the BC1 encoder does not actually support transparency in 3-color mode.
        // Don't set when encoding to BC3.
        Use3ColorBlocksForBlackPixels = 8,

        // If Use3ColorBlocks is set, the encoder can use 3-color mode for a small but noticeable gain in average quality, but lower perf.
        // If you also specify the UseLikelyTotalOrderings flag, set the total_orderings_to_try3 paramter to the number of total orderings to try.
        // Don't set when encoding to BC3.
        Use3ColorBlocks = 16,

        // Iterative will greatly increase encode time, but is very slightly higher quality.
        // Same as squish's iterative cluster fit option. Not really worth the tiny boost in quality, unless you just don't care about perf. at all.
        Iterative = 32,

        // BoundingBox enables a fast all-integer PCA approximation on 4-color blocks.
        // At level 0 options (no other flags), this is ~15% faster, and higher *average* quality.
        BoundingBox = 64,

        // Use a slightly lower quality, but ~30% faster MSE evaluation function for 4-color blocks.
        UseFasterMSEEval = 128,

        // Examine all colors to compute selectors/MSE (slower than default)
        UseFullMSEEval = 256,

        // Use 2D least squares+inset+optimal rounding (the method used in Humus's GPU texture encoding demo), instead of PCA.
        // Around 18% faster, very slightly lower average quality to better (depends on the content).
        Use2DLS = 512,

        // Use 6 power iterations vs. 4 for PCA.
        Use6PowerIters = 2048,

        // Check all total orderings - *very* slow. The encoder is not designed to be used in this way.
        Exhaustive = 8192,

        // Try 2 different ways of choosing the initial endpoints.
        TryAllInitialEndpoints = 16384,

        // Same as BoundingBox, but implemented using integer math (faster, slightly less quality)
        BoundingBoxInt = 32768,

        // Try refining the final endpoints by examining nearby colors.
        EndpointSearchRoundsShift = 22,
        EndpointSearchRoundsMask = 1023U << EndpointSearchRoundsShift,
    };

    enum class ColorMode {
        Incomplete = 0x00,
        ThreeColor = 0x03,
        FourColor = 0x04,
        UseBlack = 0x10,
        Solid = 0x20,
        ThreeColorBlack = ThreeColor | UseBlack,
        ThreeColorSolid = ThreeColor | Solid,
        FourColorSolid = FourColor | Solid,
    };

    enum class ErrorMode { None, Faster, Check2, Full };
    enum class EndpointMode { LeastSquares, BoundingBox, BoundingBoxInt, PCA };

    // Unpacked BC1 block with metadata
    struct EncodeResults {
        Color low;
        Color high;
        std::array<uint8_t, 16> selectors;
        ColorMode color_mode;
        unsigned error = UINT_MAX;
    };

    BC1Encoder(InterpolatorPtr interpolator);

    void EncodeBlock(Color4x4 pixels, BC1Block *dest) const override;

   private:
    using Hash = uint16_t;
    using BlockMetrics = Color4x4::BlockMetrics;

    const InterpolatorPtr _interpolator;

    // match tables used for single-color blocks
    // Each entry includes a high and low pair that best reproduces the 8-bit index as well as possible,
    // with an included error value
    // these depend on the interpolator
    const MatchListPtr _single_match5 = SingleColorTable<5, 4>(_interpolator);
    const MatchListPtr _single_match6 = SingleColorTable<6, 4>(_interpolator);
    const MatchListPtr _single_match5_half = SingleColorTable<5, 3>(_interpolator);
    const MatchListPtr _single_match6_half = SingleColorTable<6, 3>(_interpolator);

    Flags _flags;
    ErrorMode _error_mode;
    EndpointMode _endpoint_mode;
    unsigned _search_rounds;
    unsigned _orderings4;
    unsigned _orderings3;

    void WriteBlockSolid(Color color, BC1Block *dest) const;
    void WriteBlock(EncodeResults &block, BC1Block *dest) const;

    void FindEndpoints(Color4x4 pixels, EncodeResults &block, const BlockMetrics &metrics, EndpointMode endpoint_mode) const;
    void FindEndpointsSingleColor(EncodeResults &block, Color color, bool is_3color = false) const;
    void FindEndpointsSingleColor(EncodeResults &block, Color4x4 &pixels, Color color, bool is_3color) const;

    template <ColorMode M> void FindSelectors(Color4x4 &pixels, EncodeResults &block, ErrorMode error_mode) const;

    template <ColorMode M> bool RefineEndpointsLS(Color4x4 pixels, EncodeResults &block, BlockMetrics metrics) const;

    template <ColorMode M> void RefineEndpointsLS(std::array<Vector4, 17> &sums, EncodeResults &block, Vector4 &matrix, Hash hash) const;

    template <ColorMode M> void RefineBlockLS(Color4x4 &pixels, EncodeResults &block, BlockMetrics &metrics, ErrorMode error_mode, unsigned passes) const;

    template <ColorMode M> void RefineBlockCF(Color4x4 &pixels, EncodeResults &block, BlockMetrics &metrics, ErrorMode error_mode, unsigned orderings) const;
};
}  // namespace rgbcx
