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

#pragma once

#include <array>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <tuple>

#include "../../Color.h"
#include "../../ColorBlock.h"
#include "../../Encoder.h"
#include "../../Texture.h"
#include "../interpolator/Interpolator.h"
#include "BC1Block.h"
#include "SingleColorTable.h"

namespace quicktex {
class Vector4;
}

namespace quicktex::s3tc {

class BC1Encoder final : public BlockEncoder<BlockTexture<BC1Block>> {
   public:
    using InterpolatorPtr = std::shared_ptr<Interpolator>;
    using OrderingPair = std::tuple<unsigned, unsigned>;
    using CBlock = ColorBlock<4, 4>;

    static constexpr unsigned min_power_iterations = 4;
    static constexpr unsigned max_power_iterations = 10;

    enum class ColorMode {
        // An incomplete block with invalid selectors or endpoints
        Incomplete = 0x00,

        // A block where color0 <= color1
        ThreeColor = 0x03,

        // A block where color0 > color1
        FourColor = 0x04,

        // A 3 color block with black pixels (selector 3)
        UseBlack = 0x10,
        ThreeColorBlack = ThreeColor | UseBlack,
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

    bool exhaustive;
    bool two_ls_passes;
    bool two_ep_passes;
    bool two_cf_passes;

    BC1Encoder(unsigned level, ColorMode color_mode, InterpolatorPtr interpolator);

    BC1Encoder(unsigned int level = 5, ColorMode color_mode = ColorMode::FourColor) : BC1Encoder(level, color_mode, std::make_shared<Interpolator>()) {}

    // Getters and Setters
    void SetLevel(unsigned level);

    ErrorMode GetErrorMode() const { return _error_mode; }
    void SetErrorMode(ErrorMode error_mode) { _error_mode = error_mode; };

    EndpointMode GetEndpointMode() const { return _endpoint_mode; }
    void SetEndpointMode(EndpointMode endpoint_mode) { _endpoint_mode = endpoint_mode; }

    InterpolatorPtr GetInterpolator() const { return _interpolator; }
    ColorMode GetColorMode() const { return _color_mode; }

    unsigned int GetSearchRounds() const { return _search_rounds; }
    void SetSearchRounds(unsigned search_rounds) { _search_rounds = search_rounds; }

    unsigned GetOrderings4() const { return _orderings4; }
    unsigned GetOrderings3() const { return _orderings3; }

    void SetOrderings4(unsigned orderings4);
    void SetOrderings3(unsigned orderings3);

    OrderingPair GetOrderings() const { return OrderingPair(_orderings4, _orderings3); }
    void SetOrderings(OrderingPair orderings);

    unsigned GetPowerIterations() const { return _power_iterations; }
    void SetPowerIterations(unsigned power_iters);

    // Public Methods
    BC1Block EncodeBlock(const CBlock &pixels) const override;

    virtual size_t MTThreshold() const override { return 16; }

   private:
    using Hash = uint16_t;
    using BlockMetrics = CBlock::Metrics;

    // Unpacked BC1 block with metadata
    struct EncodeResults {
        Color low;
        Color high;
        std::array<uint8_t, 16> selectors = {0};
        ColorMode color_mode = ColorMode::Incomplete;
        bool solid = false;
        unsigned error = UINT_MAX;
    };

    const InterpolatorPtr _interpolator;
    const ColorMode _color_mode;

    // match tables used for single-color blocks
    // Each entry includes a high and low pair that best reproduces the 8-bit index as well as possible,
    // with an included error value
    // these depend on the interpolator
    MatchListPtr _single_match5 = SingleColorTable<5, 4>(_interpolator);
    MatchListPtr _single_match6 = SingleColorTable<6, 4>(_interpolator);
    MatchListPtr _single_match5_half = SingleColorTable<5, 3>(_interpolator);
    MatchListPtr _single_match6_half = SingleColorTable<6, 3>(_interpolator);

    ErrorMode _error_mode;
    EndpointMode _endpoint_mode;

    unsigned _power_iterations;
    unsigned _search_rounds;
    unsigned _orderings4;
    unsigned _orderings3;

    BC1Block WriteBlockSolid(Color color) const;
    BC1Block WriteBlock(EncodeResults &result) const;

    void FindEndpoints(EncodeResults &result, const CBlock &pixels, const BlockMetrics &metrics, EndpointMode endpoint_mode, bool ignore_black = false) const;
    void FindEndpointsSingleColor(EncodeResults &result, Color color, bool is_3color = false) const;
    void FindEndpointsSingleColor(EncodeResults &result, const CBlock &pixels, Color color, bool is_3color) const;

    template <ColorMode M> void FindSelectors(EncodeResults &result, const CBlock &pixels, ErrorMode error_mode) const;

    template <ColorMode M> bool RefineEndpointsLS(EncodeResults &result, const CBlock &pixels, BlockMetrics metrics) const;

    template <ColorMode M> void RefineEndpointsLS(EncodeResults &result, std::array<Vector4, 17> &sums, Vector4 &matrix, Hash hash) const;

    template <ColorMode M>
    void RefineBlockLS(EncodeResults &result, const CBlock &pixels, const BlockMetrics &metrics, ErrorMode error_mode, unsigned passes) const;

    template <ColorMode M>
    void RefineBlockCF(EncodeResults &result, const CBlock &pixels, const BlockMetrics &metrics, ErrorMode error_mode, unsigned orderings) const;

    void EndpointSearch(EncodeResults &result, const CBlock &pixels) const;
};
}  // namespace quicktex::s3tc
