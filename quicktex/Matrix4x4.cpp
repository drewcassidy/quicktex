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

#include "Matrix4x4.h"

namespace quicktex {

Matrix4x4 operator*(const Matrix4x4& lhs, const Matrix4x4& rhs) {
    Matrix4x4 trans_rhs = rhs.Transpose();  // 🏳️‍⚧️
    Matrix4x4 result;
    for (unsigned r = 0; r < 4; r++) {
        for (unsigned c = 0; c < 4; c++) { result[r][c] = lhs[r].Dot(trans_rhs[c]); }
    }

    return result;
}

Vector4 operator*(const Matrix4x4& lhs, const Vector4& rhs) {
    Vector4 result;

    for (unsigned r = 0; r < 4; r++) { result[r] = rhs.Dot(lhs[r]); }

    return result;
}

void Matrix4x4::Mirror() {
    for (unsigned r = 0; r < 3; r++) {
        for (unsigned c = (r + 1); c < 4; c++) {
            _r[c][r] = _r[r][c];
        }
    }
}

}  // namespace quicktex
