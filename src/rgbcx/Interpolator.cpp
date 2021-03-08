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

#include "Interpolator.h"

#include <array>
#include <cassert>
#include <cstdint>
#include <stdexcept>

#include "util.h"

namespace rgbcx {

/*
Interpolator::Interpolator() {
    PrepSingleColorTables(_single_match5, _single_match5_half, 5);
    PrepSingleColorTables(_single_match5, _single_match5_half, 6);
}

void Interpolator::PrepSingleColorTables(const MatchListPtr &matchTable, const MatchListPtr &matchTableHalf, int len) {
    int size = 1 << len;

    assert((len == 5 && size == Size5) || (len == 6 && size == size6));

    const uint8_t *expand = (len == 5) ? &Expand5[0] : &Expand6[0];

    bool ideal = IsIdeal();
    bool use_e = useExpandedInMatch();

    for (int i = 0; i < match_count; i++) {
        int lowest_error = 256;
        int lowest_half_error = 256;

        for (int low = 0; low < size; low++) {
            const int low_e = expand[low];
            const int low_val = use_e ? low_e : low;

            for (int high = 0; high < size; high++) {
                const int high_e = expand[high];
                const int high_val = use_e ? high_e : high;

                int v = (len == 5) ? Interpolate5(high_val, low_val) : Interpolate6(high_val, low_val);
                int v_half = (len == 5) ? InterpolateHalf5(low_val, high_val) : InterpolateHalf6(low_val, high_val);

                int error = PrepSingleColorTableEntry(matchTable, v, i, low, high, low_e, high_e, lowest_error, false, ideal);
                int half_error = PrepSingleColorTableEntry(matchTableHalf, v, i, low, high, low_e, high_e, lowest_error, true, ideal);

                if (error < lowest_error) lowest_error = error;
                if (half_error < lowest_half_error) lowest_half_error = half_error;
            }
        }
    }
}
int Interpolator::PrepSingleColorTableEntry(const MatchListPtr &matchTable, int v, int i, int low, int high, int low_e, int high_e, int lowest_error, bool half,
                                            bool ideal) {
    int e = iabs(v - i);

    // We only need to factor in 3% error in BC1 ideal mode.
    if (ideal) e += (iabs(high_e - low_e) * 3) / 100;

    // Favor equal endpoints, for lower error on actual GPU's which approximate the interpolation.
    if ((e < lowest_error) || (e == lowest_error && low == high)) {
        assert(e <= UINT8_MAX);

        auto &entry = (*matchTable)[i];
        entry.low = low;
        entry.high = high;
        entry.error = e;
    }

    return e;
}*/

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

std::array<Color, 4> Interpolator::InterpolateBC1(uint16_t low, uint16_t high) const {
    return InterpolateBC1(Color::Unpack565Unscaled(low), Color::Unpack565Unscaled(high), (high >= low));
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
}  // namespace rgbcx
