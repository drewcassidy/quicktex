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
#include <cmath>
#include <functional>

#include "Color.h"

namespace quicktex {

class Vector4 {
   public:
    Vector4() : Vector4(0) {}

    Vector4(float x, float y, float z = 0, float w = 0) {
        _c[0] = x;
        _c[1] = y;
        _c[2] = z;
        _c[3] = w;
    }

    Vector4(float scalar) {
        _c[0] = scalar;
        _c[1] = scalar;
        _c[2] = scalar;
        _c[3] = scalar;
    }

    Vector4(const Color &c) : Vector4(c.r, c.g, c.b, c.a) {}

    static Vector4 FromColor(const Color &c) { return Vector4(c); }

    static Vector4 FromColorRGB(const Color &c) { return Vector4(c.r, c.g, c.b); }

    static float Dot(const Vector4 &lhs, const Vector4 &rhs) {
        float sum = 0;
        for (unsigned i = 0; i < 4; i++) { sum += lhs[i] * rhs[i]; }
        return sum;
    }

    float operator[](size_t index) const {
        assert(index < 4);
        return _c[index];
    }
    float &operator[](size_t index) {
        assert(index < 4);
        return _c[index];
    }

    friend Vector4 operator+(const Vector4 &lhs, const Vector4 &rhs) { return DoOp(lhs, rhs, std::plus()); }
    friend Vector4 operator-(const Vector4 &lhs, const Vector4 &rhs) { return DoOp(lhs, rhs, std::minus()); }
    friend Vector4 operator*(const Vector4 &lhs, const Vector4 &rhs) { return DoOp(lhs, rhs, std::multiplies()); }
    friend Vector4 operator/(const Vector4 &lhs, const Vector4 &rhs) { return DoOp(lhs, rhs, std::divides()); }

    friend Vector4 operator+(const Vector4 &lhs, const float &rhs) { return DoOp(lhs, rhs, std::plus()); }
    friend Vector4 operator-(const Vector4 &lhs, const float &rhs) { return DoOp(lhs, rhs, std::minus()); }
    friend Vector4 operator*(const Vector4 &lhs, const float &rhs) { return DoOp(lhs, rhs, std::multiplies()); }
    friend Vector4 operator/(const Vector4 &lhs, const float &rhs) { return DoOp(lhs, rhs, std::divides()); }

    friend Vector4 &operator+=(Vector4 &lhs, const Vector4 &rhs) { return lhs = lhs + rhs; }
    friend Vector4 &operator-=(Vector4 &lhs, const Vector4 &rhs) { return lhs = lhs - rhs; }
    friend Vector4 &operator*=(Vector4 &lhs, const Vector4 &rhs) { return lhs = lhs * rhs; }
    friend Vector4 &operator/=(Vector4 &lhs, const Vector4 &rhs) { return lhs = lhs / rhs; }

    friend Vector4 &operator+=(Vector4 &lhs, const float &rhs) { return lhs = lhs + rhs; }
    friend Vector4 &operator-=(Vector4 &lhs, const float &rhs) { return lhs = lhs - rhs; }
    friend Vector4 &operator*=(Vector4 &lhs, const float &rhs) { return lhs = lhs * rhs; }
    friend Vector4 &operator/=(Vector4 &lhs, const float &rhs) { return lhs = lhs / rhs; }

    float Dot(Vector4 other) const { return Dot(*this, other); }
    float MaxAbs(unsigned channels = 4) const {
        assert(channels < 5);
        assert(channels > 0);
        float max = 0;
        for (unsigned i = 0; i < channels; i++) {
            float a = fabs((*this)[i]);
            if (a > max) max = a;
        }
        return max;
    }

    float SqrMag() { return Dot(*this, *this); }

    float Determinant2x2() {
        //z00 * z11 - z01 * z10;
        return (_c[0] * _c[3]) - (_c[1] * _c[2]);
    }

   private:
    template <typename Op> static inline Vector4 DoOp(const Vector4 &lhs, const Vector4 &rhs, Op f) {
        Vector4 r;
        for (unsigned i = 0; i < 4; i++) { r[i] = f(lhs[i], rhs[i]); }
        return r;
    }

    template <typename Op> static inline Vector4 DoOp(const Vector4 &lhs, const float &rhs, Op f) {
        Vector4 r;
        for (unsigned i = 0; i < 4; i++) { r[i] = f(lhs[i], rhs); }
        return r;
    }

    std::array<float, 4> _c;
};

}  // namespace quicktex
