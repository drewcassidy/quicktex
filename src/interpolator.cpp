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

#include <cassert>

// region InterpolatorIdeal implementation
int rgbcx::InterpolatorIdeal::Interpolate5(int v0, int v1) { return Interpolate5or6(v0, v1); }
int rgbcx::InterpolatorIdeal::Interpolate6(int v0, int v1) { return Interpolate5or6(v0, v1); }
int rgbcx::InterpolatorIdeal::InterpolateHalf5(int v0, int v1) { return InterpolateHalf5or6(v0, v1); }
int rgbcx::InterpolatorIdeal::InterpolateHalf6(int v0, int v1) { return InterpolateHalf5or6(v0, v1); }

int rgbcx::InterpolatorIdeal::Interpolate5or6(int v0, int v1) {
    assert(v0 < 256 && v1 < 256);
    return (v0 * 2 + v1) / 3;
}

int rgbcx::InterpolatorIdeal::InterpolateHalf5or6(int v0, int v1) {
    assert(v0 < 256 && v1 < 256);
    return (v0 + v1) / 2;
}
// endregion

// region InterpolatorIdealRound implementation
int rgbcx::InterpolatorIdealRound::Interpolate5(int v0, int v1) { return Interpolate5or6Round(v0, v1); }
int rgbcx::InterpolatorIdealRound::Interpolate6(int v0, int v1) { return Interpolate5or6Round(v0, v1); }

int rgbcx::InterpolatorIdealRound::Interpolate5or6Round(int v0, int v1) {
    assert(v0 < 256 && v1 < 256);
    return (v0 * 2 + v1 + 1) / 3;
}
// endregion

// region InterpolatorNvidia implementation
int rgbcx::InterpolatorNvidia::Interpolate5(int v0, int v1) {
    assert(v0 < 32 && v1 < 32);
    return ((2 * v0 + v1) * 22) / 8;
}

int rgbcx::InterpolatorNvidia::Interpolate6(int v0, int v1) {
    assert(v0 < 256 && v1 < 256);
    const int gdiff = v1 - v0;
    return (256 * v0 + (gdiff / 4) + 128 + gdiff * 80) / 256;
}

int rgbcx::InterpolatorNvidia::InterpolateHalf5(int v0, int v1) {
    assert(v0 < 32 && v1 < 32);
    return ((v0 + v1) * 33) / 8;
}

int rgbcx::InterpolatorNvidia::InterpolateHalf6(int v0, int v1) {
    assert(v0 < 256 && v1 < 256);
    const int gdiff = v1 - v0;
    return (256 * v0 + gdiff / 4 + 128 + gdiff * 128) / 256;
}
// endregion

// region InterpolatorAMD implementation
int rgbcx::InterpolatorAMD::Interpolate5(int v0, int v1) { return Interpolate5or6(v0, v1); }
int rgbcx::InterpolatorAMD::Interpolate6(int v0, int v1) { return Interpolate5or6(v0, v1); }
int rgbcx::InterpolatorAMD::InterpolateHalf5(int v0, int v1) { return InterpolateHalf5or6(v0, v1); }
int rgbcx::InterpolatorAMD::InterpolateHalf6(int v0, int v1) { return InterpolateHalf5or6(v0, v1); }

int rgbcx::InterpolatorAMD::Interpolate5or6(int v0, int v1) {
    assert(v0 < 256 && v1 < 256);
    return (v0 * 43 + v1 * 21 + 32) >> 6;
}

int rgbcx::InterpolatorAMD::InterpolateHalf5or6(int v0, int v1) {
    assert(v0 < 256 && v1 < 256);
    return (v0 + v1 + 1) >> 1;
}
// endregion
