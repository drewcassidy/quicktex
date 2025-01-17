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
#include <functional>

#include "Color.h"
#include "Vector4.h"

namespace quicktex {

class Vector4Int {
   public:
    Vector4Int() : Vector4Int(0) {}

    Vector4Int(int x, int y, int z = 0, int w = 0) {
        _c[0] = x;
        _c[1] = y;
        _c[2] = z;
        _c[3] = w;
    }

    Vector4Int(int scalar) {
        _c[0] = scalar;
        _c[1] = scalar;
        _c[2] = scalar;
        _c[3] = scalar;
    }

    Vector4Int(const Color &c) : Vector4Int(c.r, c.g, c.b, c.a) {}

    static Vector4Int FromColor(const Color &c) { return Vector4Int(c); }

    static Vector4Int FromColorRGB(const Color &c) { return Vector4Int(c.r, c.g, c.b); }

    static int Dot(const Vector4Int &lhs, const Vector4Int &rhs) {
        int sum = 0;
        for (unsigned i = 0; i < 4; i++) { sum += lhs[i] * rhs[i]; }
        return sum;
    }

    int operator[](size_t index) const {
        assert(index < 4);
        return _c[index];
    }
    int &operator[](size_t index) {
        assert(index < 4);
        return _c[index];
    }

    operator Vector4() const { return Vector4((float)_c[0], (float)_c[1], (float)_c[2], (float)_c[3]); }

    friend Vector4Int operator+(const Vector4Int &lhs, const Vector4Int &rhs) { return DoOp(lhs, rhs, std::plus()); }
    friend Vector4Int operator-(const Vector4Int &lhs, const Vector4Int &rhs) { return DoOp(lhs, rhs, std::minus()); }
    friend Vector4Int operator*(const Vector4Int &lhs, const Vector4Int &rhs) { return DoOp(lhs, rhs, std::multiplies()); }
    friend Vector4Int operator/(const Vector4Int &lhs, const Vector4Int &rhs) { return DoOp(lhs, rhs, std::divides()); }

    friend Vector4Int operator+(const Vector4Int &lhs, const int &rhs) { return DoOp(lhs, rhs, std::plus()); }
    friend Vector4Int operator-(const Vector4Int &lhs, const int &rhs) { return DoOp(lhs, rhs, std::minus()); }
    friend Vector4Int operator*(const Vector4Int &lhs, const int &rhs) { return DoOp(lhs, rhs, std::multiplies()); }
    friend Vector4Int operator/(const Vector4Int &lhs, const int &rhs) { return DoOp(lhs, rhs, std::divides()); }

    friend Vector4Int &operator+=(Vector4Int &lhs, const Vector4Int &rhs) { return lhs = lhs + rhs; }
    friend Vector4Int &operator-=(Vector4Int &lhs, const Vector4Int &rhs) { return lhs = lhs - rhs; }
    friend Vector4Int &operator*=(Vector4Int &lhs, const Vector4Int &rhs) { return lhs = lhs * rhs; }
    friend Vector4Int &operator/=(Vector4Int &lhs, const Vector4Int &rhs) { return lhs = lhs / rhs; }

    friend Vector4Int &operator+=(Vector4Int &lhs, const int &rhs) { return lhs = lhs + rhs; }
    friend Vector4Int &operator-=(Vector4Int &lhs, const int &rhs) { return lhs = lhs - rhs; }
    friend Vector4Int &operator*=(Vector4Int &lhs, const int &rhs) { return lhs = lhs * rhs; }
    friend Vector4Int &operator/=(Vector4Int &lhs, const int &rhs) { return lhs = lhs / rhs; }

    int Dot(const Vector4Int &other) const { return Dot(*this, other); }
    int MaxAbs(unsigned channels = 4) {
        assert(channels < 5);
        assert(channels > 0);
        int max = 0;
        for (unsigned i = 0; i < channels; i++) {
            int a = abs((*this)[i]);
            if (a > max) max = a;
        }
        return max;
    }
    unsigned int SqrMag() { return (unsigned)Dot(*this, *this); }

   private:
    template <typename Op> friend Vector4Int DoOp(const Vector4Int &lhs, const Vector4Int &rhs, Op f) {
        Vector4Int r;
        for (unsigned i = 0; i < 4; i++) { r[i] = f(lhs[i], rhs[i]); }
        return r;
    }

    template <typename Op> friend Vector4Int DoOp(const Vector4Int &lhs, const int &rhs, Op f) {
        Vector4Int r;
        for (unsigned i = 0; i < 4; i++) { r[i] = f(lhs[i], rhs); }
        return r;
    }

    std::array<int, 4> _c;
};

}  // namespace quicktex
