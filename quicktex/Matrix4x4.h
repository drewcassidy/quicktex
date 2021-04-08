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
