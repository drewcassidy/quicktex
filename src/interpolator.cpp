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

#include "interpolator.h"

#include <array>
#include <cassert>
#include <cstdint>

#include "util.h"

namespace rgbcx {

Interpolator::Interpolator() {
    PrepSingleColorTables(_single_match5, _single_match5_half, 5);
    PrepSingleColorTables(_single_match5, _single_match5_half, 6);
}

void Interpolator::PrepSingleColorTables(const MatchListPtr &matchTable, const MatchListPtr &matchTableHalf, int len) {
    int size = 1 << len;

    assert((len == 5 && size == size5) || (len == 6 && size == size6));

    const uint8_t *expand = (len == 5) ? &Expand5[0] : &Expand6[0];

    bool ideal = isIdeal();
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
int Interpolator::PrepSingleColorTableEntry(const MatchListPtr &matchTable, int v, int i, int low, int high, int low_e, int high_e, int lowest_error,
                                                 bool half, bool ideal) {
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
}

// region InterpolatorIdeal implementation
int InterpolatorIdeal::Interpolate5(int v0, int v1) const { return Interpolate5or6(v0, v1); }
int InterpolatorIdeal::Interpolate6(int v0, int v1) const { return Interpolate5or6(v0, v1); }
int InterpolatorIdeal::InterpolateHalf5(int v0, int v1) const { return InterpolateHalf5or6(v0, v1); }
int InterpolatorIdeal::InterpolateHalf6(int v0, int v1) const { return InterpolateHalf5or6(v0, v1); }

int InterpolatorIdeal::Interpolate5or6(int v0, int v1) const {
    assert(v0 < 256 && v1 < 256);
    return (v0 * 2 + v1) / 3;
}

int InterpolatorIdeal::InterpolateHalf5or6(int v0, int v1) const {
    assert(v0 < 256 && v1 < 256);
    return (v0 + v1) / 2;
}
// endregion

// region InterpolatorIdealRound implementation
int InterpolatorIdealRound::Interpolate5(int v0, int v1) const { return Interpolate5or6Round(v0, v1); }
int InterpolatorIdealRound::Interpolate6(int v0, int v1) const { return Interpolate5or6Round(v0, v1); }

int InterpolatorIdealRound::Interpolate5or6Round(int v0, int v1) const {
    assert(v0 < 256 && v1 < 256);
    return (v0 * 2 + v1 + 1) / 3;
}
// endregion

// region InterpolatorNvidia implementation
int InterpolatorNvidia::Interpolate5(int v0, int v1) const {
    assert(v0 < 32 && v1 < 32);
    return ((2 * v0 + v1) * 22) / 8;
}

int InterpolatorNvidia::Interpolate6(int v0, int v1) const {
    assert(v0 < 256 && v1 < 256);
    const int gdiff = v1 - v0;
    return (256 * v0 + (gdiff / 4) + 128 + gdiff * 80) / 256;
}

int InterpolatorNvidia::InterpolateHalf5(int v0, int v1) const {
    assert(v0 < 32 && v1 < 32);
    return ((v0 + v1) * 33) / 8;
}

int InterpolatorNvidia::InterpolateHalf6(int v0, int v1) const {
    assert(v0 < 256 && v1 < 256);
    const int gdiff = v1 - v0;
    return (256 * v0 + gdiff / 4 + 128 + gdiff * 128) / 256;
}
// endregion

// region InterpolatorAMD implementation
int InterpolatorAMD::Interpolate5(int v0, int v1) const { return Interpolate5or6(v0, v1); }
int InterpolatorAMD::Interpolate6(int v0, int v1) const { return Interpolate5or6(v0, v1); }
int InterpolatorAMD::InterpolateHalf5(int v0, int v1) const { return InterpolateHalf5or6(v0, v1); }
int InterpolatorAMD::InterpolateHalf6(int v0, int v1) const { return InterpolateHalf5or6(v0, v1); }

int InterpolatorAMD::Interpolate5or6(int v0, int v1) const {
    assert(v0 < 256 && v1 < 256);
    return (v0 * 43 + v1 * 21 + 32) >> 6;
}

int InterpolatorAMD::InterpolateHalf5or6(int v0, int v1) const {
    assert(v0 < 256 && v1 < 256);
    return (v0 + v1 + 1) >> 1;
}
// endregion
}  // namespace rgbcx
