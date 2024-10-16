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

#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <functional>

#include "Vector4.h"

namespace quicktex {

class Matrix4x4 {
   public:
    static Matrix4x4 Identity() {
        Matrix4x4 result;
        for (unsigned i = 0; i < 4; i++) { result[i][i] = 1; }
        return result;
    }

    static Matrix4x4 Transpose(const Matrix4x4 &val) {
        Matrix4x4 result;
        for (unsigned r = 0; r < 3; r++) {
            for (unsigned c = 0; c < 3; c++) { result[r][c] = val[c][r]; }
        }
        return result;
    }

    Vector4 operator[](size_t index) const {
        assert(index < 4);
        return _r[index];
    }
    Vector4 &operator[](size_t index) {
        assert(index < 4);
        return _r[index];
    }

    friend Matrix4x4 operator*(const Matrix4x4 &lhs, const Matrix4x4 &rhs);
    friend Vector4 operator*(const Matrix4x4 &lhs, const Vector4 &rhs);

    friend Matrix4x4 operator+(const Matrix4x4 &lhs, const Matrix4x4 &rhs) { return DoOp(lhs, rhs, std::plus()); }
    friend Matrix4x4 operator-(const Matrix4x4 &lhs, const Matrix4x4 &rhs) { return DoOp(lhs, rhs, std::minus()); }

    friend Matrix4x4 operator+(const Matrix4x4 &lhs, const float &rhs) { return DoOp(lhs, rhs, std::plus()); }
    friend Matrix4x4 operator-(const Matrix4x4 &lhs, const float &rhs) { return DoOp(lhs, rhs, std::minus()); }
    friend Matrix4x4 operator*(const Matrix4x4 &lhs, const float &rhs) { return DoOp(lhs, rhs, std::multiplies()); }
    friend Matrix4x4 operator/(const Matrix4x4 &lhs, const float &rhs) { return DoOp(lhs, rhs, std::divides()); }

    friend Matrix4x4 &operator+=(Matrix4x4 &lhs, const Matrix4x4 &rhs) { return lhs = lhs + rhs; }
    friend Matrix4x4 &operator-=(Matrix4x4 &lhs, const Matrix4x4 &rhs) { return lhs = lhs - rhs; }
    friend Matrix4x4 &operator*=(Matrix4x4 &lhs, const Matrix4x4 &rhs) { return lhs = lhs * rhs; }

    friend Matrix4x4 &operator+=(Matrix4x4 &lhs, const float &rhs) { return lhs = lhs + rhs; }
    friend Matrix4x4 &operator-=(Matrix4x4 &lhs, const float &rhs) { return lhs = lhs - rhs; }
    friend Matrix4x4 &operator*=(Matrix4x4 &lhs, const float &rhs) { return lhs = lhs * rhs; }
    friend Matrix4x4 &operator/=(Matrix4x4 &lhs, const float &rhs) { return lhs = lhs / rhs; }

    Matrix4x4 Transpose() const { return Transpose(*this); }

    void Mirror();

   private:
    template <typename Op> friend Matrix4x4 DoOp(const Matrix4x4 &lhs, const Matrix4x4 &rhs, Op f) {
        Matrix4x4 result;
        for (unsigned r = 0; r < 4; r++) { result[r] = f(lhs[r], rhs[r]); }
        return result;
    }

    template <typename Op> friend Matrix4x4 DoOp(const Matrix4x4 &lhs, const float &rhs, Op f) {
        Matrix4x4 result;
        for (unsigned r = 0; r < 4; r++) { result[r] = f(lhs[r], rhs); }
        return result;
    }

    std::array<Vector4, 4> _r;
};
}  // namespace quicktex
