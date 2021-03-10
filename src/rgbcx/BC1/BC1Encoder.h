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

class BC1Encoder final : public BlockEncoderTemplate<BC1Block, 4, 4> {
   public:
    using InterpolatorPtr = std::shared_ptr<Interpolator>;

    enum class Flags {
        None = 0,

        // Try to improve quality using the most likely total orderings.
        // The total_orderings_to_try parameter will then control the number of total orderings to try for 4 color blocks, and the
        // total_orderings_to_try3 parameter will control the number of total orderings to try for 3 color blocks (if they are enabled).
        UseLikelyTotalOrderings = 1,

        // Use 2 least squares pass, instead of one (same as stb_dxt's HIGHQUAL option).
        // Recommended if you're enabling UseLikelyTotalOrderings.
        TwoLeastSquaresPasses = 2,

        // Use3ColorBlocksForBlackPixels allows the BC1 encoder to use 3-color blocks for blocks containing black or very dark pixels.
        // You shader/engine MUST ignore the alpha channel on textures encoded with this flag.
        // Average quality goes up substantially for my 100 texture corpus (~.5 dB), so it's worth using if you can.
        // Note the BC1 encoder does not actually support transparency in 3-color mode.
        // Don't set when encoding to BC3.
        Use3ColorBlocksForBlackPixels = 4,

        // If Use3ColorBlocks is set, the encoder can use 3-color mode for a small but noticeable gain in average quality, but lower perf.
        // If you also specify the UseLikelyTotalOrderings flag, set the total_orderings_to_try3 paramter to the number of total orderings to try.
        // Don't set when encoding to BC3.
        Use3ColorBlocks = 8,

        // Iterative will greatly increase encode time, but is very slightly higher quality.
        // Same as squish's iterative cluster fit option. Not really worth the tiny boost in quality, unless you just don't care about perf. at all.
        Iterative = 16,

        // Use 6 power iterations vs. 4 for PCA.
        Use6PowerIters = 32,

        // Check all total orderings - *very* slow. The encoder is not designed to be used in this way.
        Exhaustive = 64,

        // Try 2 different ways of choosing the initial endpoints.
        TryAllInitialEndpoints = 128,
    };

    enum class ErrorMode {
        // Perform no error checking at all.
        None,

        // Use a slightly lower quality, but ~30% faster MSE evaluation function for 4-color blocks.
        Faster,

        // Default error mode.
        Check2,

        // Examine all colors to compute selectors/MSE (slower than default).
        Full
    };

    enum class EndpointMode {
        // Use 2D least squares+inset+optimal rounding (the method used in Humus's GPU texture encoding demo), instead of PCA.
        // Around 18% faster, very slightly lower average quality to better (depends on the content).
        LeastSquares,

        // BoundingBox enables a fast all-integer PCA approximation on 4-color blocks.
        // At level 0 options (no other flags), this is ~15% faster, and higher *average* quality.
        BoundingBox,

        // Same as BoundingBox, but implemented using integer math (faster, slightly less quality)
        BoundingBoxInt,

        // Full PCA implementation
        PCA
    };

    BC1Encoder(Interpolator::Type type = Interpolator::Type::Ideal, unsigned level = 5, bool allow_3color = true, bool allow_3color_black = true);

    Interpolator::Type GetInterpolatorType() const { return _interpolator->GetType(); }

    void SetLevel(unsigned level, bool allow_3color = true, bool allow_3color_black = true);

    Flags GetFlags() const { return _flags; }
    void SetFlags(Flags flags) { _flags = flags; };

    ErrorMode GetErrorMode() const { return _error_mode; }
    void SetErrorMode(ErrorMode error_mode) { _error_mode = error_mode; };

    EndpointMode GetEndpointMode() const { return _endpoint_mode; }
    void SetEndpointMode(EndpointMode endpoint_mode) { _endpoint_mode = endpoint_mode; }

    unsigned int GetSearchRounds() const { return _search_rounds; }
    void SetSearchRounds(unsigned search_rounds) { _search_rounds = search_rounds; }

    unsigned int GetOrderings4() const { return _orderings4; }
    unsigned int GetOrderings3() const { return _orderings3; }
    void SetOrderings4(unsigned orderings4);
    void SetOrderings3(unsigned orderings3);

    void EncodeBlock(Color4x4 pixels, BC1Block *dest) const override;

    virtual size_t MTThreshold() const override { return 16; }

   private:
    using Hash = uint16_t;
    using BlockMetrics = Color4x4::BlockMetrics;

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

    // Unpacked BC1 block with metadata
    struct EncodeResults {
        Color low;
        Color high;
        std::array<uint8_t, 16> selectors;
        ColorMode color_mode;
        unsigned error = UINT_MAX;
    };

    const InterpolatorPtr _interpolator;

    // match tables used for single-color blocks
    // Each entry includes a high and low pair that best reproduces the 8-bit index as well as possible,
    // with an included error value
    // these depend on the interpolator
    const BC1::MatchListPtr _single_match5 = BC1::SingleColorTable<5, 4>(_interpolator);
    const BC1::MatchListPtr _single_match6 = BC1::SingleColorTable<6, 4>(_interpolator);
    const BC1::MatchListPtr _single_match5_half = BC1::SingleColorTable<5, 3>(_interpolator);
    const BC1::MatchListPtr _single_match6_half = BC1::SingleColorTable<6, 3>(_interpolator);

    Flags _flags;
    ErrorMode _error_mode;
    EndpointMode _endpoint_mode;
    unsigned _search_rounds;
    unsigned _orderings4;
    unsigned _orderings3;

    void WriteBlockSolid(Color color, BC1Block *dest) const;
    void WriteBlock(EncodeResults &block, BC1Block *dest) const;

    void FindEndpoints(Color4x4 pixels, EncodeResults &block, const BlockMetrics &metrics, EndpointMode endpoint_mode, bool ignore_black = false) const;
    void FindEndpointsSingleColor(EncodeResults &block, Color color, bool is_3color = false) const;
    void FindEndpointsSingleColor(EncodeResults &block, Color4x4 &pixels, Color color, bool is_3color) const;

    template <ColorMode M> void FindSelectors(Color4x4 &pixels, EncodeResults &block, ErrorMode error_mode) const;

    template <ColorMode M> bool RefineEndpointsLS(Color4x4 pixels, EncodeResults &block, BlockMetrics metrics) const;

    template <ColorMode M> void RefineEndpointsLS(std::array<Vector4, 17> &sums, EncodeResults &block, Vector4 &matrix, Hash hash) const;

    template <ColorMode M> void RefineBlockLS(Color4x4 &pixels, EncodeResults &block, BlockMetrics &metrics, ErrorMode error_mode, unsigned passes) const;

    template <ColorMode M> void RefineBlockCF(Color4x4 &pixels, EncodeResults &block, BlockMetrics &metrics, ErrorMode error_mode, unsigned orderings) const;

    void EndpointSearch(Color4x4 &pixels, EncodeResults &block) const;
};
}  // namespace rgbcx
