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

#include "Interpolator.h"

#include <array>
#include <cassert>
#include <cstdint>
#include <stdexcept>

#include "../../util.h"
#include "../../Color.h"

namespace quicktex::s3tc {

// region Interpolator implementation
std::unique_ptr<Interpolator> Interpolator::MakeInterpolator(Interpolator::Type type) {
    switch (type) {
        case Type::Ideal:
            return std::make_unique<Interpolator>();
        case Type::IdealRound:
            return std::make_unique<InterpolatorRound>();
        case Type::Nvidia:
            return std::make_unique<InterpolatorNvidia>();
        case Type::AMD:
            return std::make_unique<InterpolatorAMD>();
        default:
            throw std::invalid_argument("Invalid interpolator type");
    }
}

uint8_t Interpolator::Interpolate5(uint8_t v0, uint8_t v1) const { return Interpolate8(scale5To8(v0), scale5To8(v1)); }
uint8_t Interpolator::Interpolate6(uint8_t v0, uint8_t v1) const { return Interpolate8(scale6To8(v0), scale6To8(v1)); }
uint8_t Interpolator::InterpolateHalf5(uint8_t v0, uint8_t v1) const { return InterpolateHalf8(scale5To8(v0), scale5To8(v1)); }
uint8_t Interpolator::InterpolateHalf6(uint8_t v0, uint8_t v1) const { return InterpolateHalf8(scale6To8(v0), scale6To8(v1)); }

std::array<Color, 4> Interpolator::Interpolate565BC1(uint16_t low, uint16_t high, bool allow_3color) const {
    bool use_3color = allow_3color && (high >= low);
    return InterpolateBC1(Color::Unpack565Unscaled(low), Color::Unpack565Unscaled(high), use_3color);
}

std::array<Color, 4> Interpolator::InterpolateBC1(Color low, Color high, bool use_3color) const {
    auto colors = std::array<Color, 4>();
    colors[0] = low.ScaleFrom565();
    colors[1] = high.ScaleFrom565();

    if (use_3color) {
        // 3-color mode
        colors[2] = InterpolateHalfColor24(colors[0], colors[1]);
        colors[3] = Color(0, 0, 0, 0);  // transparent black
    } else {
        // 4-color mode
        colors[2] = InterpolateColor24(colors[0], colors[1]);
        colors[3] = InterpolateColor24(colors[1], colors[0]);
    }

    return colors;
}

uint8_t Interpolator::Interpolate8(uint8_t v0, uint8_t v1) const { return (v0 * 2 + v1) / 3; }

uint8_t Interpolator::InterpolateHalf8(uint8_t v0, uint8_t v1) const { return (v0 + v1) / 2; }
// endregion

// region InterpolatorRound implementation
uint8_t InterpolatorRound::Interpolate5(uint8_t v0, uint8_t v1) const { return Interpolate8(scale5To8(v0), scale5To8(v1)); }
uint8_t InterpolatorRound::Interpolate6(uint8_t v0, uint8_t v1) const { return Interpolate8(scale6To8(v0), scale6To8(v1)); }

uint8_t InterpolatorRound::Interpolate8(uint8_t v0, uint8_t v1) const { return (v0 * 2 + v1 + 1) / 3; }
// endregion

// region InterpolatorNvidia implementation
uint8_t InterpolatorNvidia::Interpolate5(uint8_t v0, uint8_t v1) const {
    assert(v0 < 32 && v1 < 32);
    return ((2 * v0 + v1) * 22) / 8U;
}

uint8_t InterpolatorNvidia::Interpolate6(uint8_t v0, uint8_t v1) const {
    assert(v0 < 64 && v1 < 64);
    const int gdiff = (int)v1 - v0;
    return static_cast<uint8_t>((256 * v0 + (gdiff / 4) + 128 + gdiff * 80) >> 8);
}

uint8_t InterpolatorNvidia::InterpolateHalf5(uint8_t v0, uint8_t v1) const {
    assert(v0 < 32 && v1 < 32);
    return ((v0 + v1) * 33) / 8U;
}

uint8_t InterpolatorNvidia::InterpolateHalf6(uint8_t v0, uint8_t v1) const {
    assert(v0 < 64 && v1 < 64);
    const int gdiff = (int)v1 - v0;
    return static_cast<uint8_t>((256 * v0 + gdiff / 4 + 128 + gdiff * 128) >> 8);
}

std::array<Color, 4> InterpolatorNvidia::InterpolateBC1(Color low, Color high, bool use_3color) const {
    // Nvidia is special and interpolation cant be done with 8-bit values, so we need to override the default behavior
    std::array<Color, 4> colors;
    colors[0] = low.ScaleFrom565();
    colors[1] = high.ScaleFrom565();

    if (!use_3color) {
        // 4-color mode
        colors[2] = InterpolateColor565(low, high);
        colors[3] = InterpolateColor565(high, low);
    } else {
        // 3-color mode
        colors[2] = InterpolateHalfColor565(low, high);
        colors[3] = Color(0, 0, 0, 0);  // transparent black
    }

    return colors;
}
// endregion

// region InterpolatorAMD implementation
uint8_t InterpolatorAMD::Interpolate5(uint8_t v0, uint8_t v1) const { return Interpolate8(scale5To8(v0), scale5To8(v1)); }
uint8_t InterpolatorAMD::Interpolate6(uint8_t v0, uint8_t v1) const { return Interpolate8(scale6To8(v0), scale6To8(v1)); }
uint8_t InterpolatorAMD::InterpolateHalf5(uint8_t v0, uint8_t v1) const { return InterpolateHalf8(scale5To8(v0), scale5To8(v1)); }
uint8_t InterpolatorAMD::InterpolateHalf6(uint8_t v0, uint8_t v1) const { return InterpolateHalf8(scale6To8(v0), scale6To8(v1)); }

uint8_t InterpolatorAMD::Interpolate8(uint8_t v0, uint8_t v1) const { return (v0 * 43 + v1 * 21 + 32) >> 6; }

uint8_t InterpolatorAMD::InterpolateHalf8(uint8_t v0, uint8_t v1) const { return (v0 + v1 + 1) >> 1; }
// endregion
}  // namespace quicktex::s3tc
