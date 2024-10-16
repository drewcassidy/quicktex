/*  Quicktex Texture Compression Library
    Copyright (C) 2021-2024 Andrew Cassidy <drewcassidy@me.com>
    Partially derived from rgbcx.h written by Richard Geldreich <richgel99@gmail.com>
    and licenced under the public domain

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
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
