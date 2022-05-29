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

#include <cstdint>
#include <numeric>
#include <xsimd/xsimd.hpp>

#include "util.h"

namespace quicktex {

template <typename T, size_t N> class Vec {
   public:
    // region constructors
    /**
     * Create a vector from an intializer list
     * @param vals values to populate with
     */
    Vec(std::initializer_list<T> vals) { std::copy(vals.begin(), vals.end(), _c.begin()); }

    /**
     * Create a vector from a scalar value
     * @param scalar value to populate with
     */
    Vec(const T &scalar = 0) { _c.fill(scalar); }

    /**
     * Create a vector from another vector of the same size and another type
     * @tparam S Source vector type
     * @param rvalue Source vector to copy from
     */
    template <typename S> Vec(std::enable_if_t<std::is_convertible_v<S, T>, const Vec<S, N>> &rvalue) {
        for (unsigned i = 0; i < N; i++) { at(i) = static_cast<T>(rvalue[i]); }
    }

    /**
     * Create a vector from a naked pointer
     * @tparam S Source data type
     * @param ptr Pointer to the start of the source data. N values will be read.
     */
    template <typename S> Vec(const S *ptr) {
        for (unsigned i = 0; i < N; i++) { at(i) = static_cast<T>(ptr[i]); }
    }

    /**
     * Create a vector from a std::array
     * @tparam S Source data type
     * @param arr Array to copy from
     */
    template <typename S> Vec(const std::array<S, N> &arr) : Vec(arr.begin()) {}
    // endregion

    // region subscript accessors
    /**
     * Get the element at index i
     * @param i index to read from
     * @return the element at index i
     */
    T at(size_t i) const {
        assert(i < N);
        return _c[i];
    }

    /**
     * Get a reference to the element at index i
     * @param i index to read from
     * @return Reference to the element at index i
     */
    T &at(size_t i) {
        assert(i < N);
        return _c[i];
    }

    /**
     * Get the element at index i
     * @param i index to read from
     * @return the element at index i
     */
    T operator[](size_t i) const { return at(i); }

    /**
     * Get a reference to the element at index i
     * @param i index to read from
     * @return Reference to the element at index i
     */
    T &operator[](size_t i) { return at(i); }

    T *begin() { return _c.begin(); }
    T *end() { return _c.end(); }
    const T *begin() const { return _c.begin(); }
    const T *end() const { return _c.end(); }

    // endregion

    // region accessor shortcuts
    // RGBA accessors
    std::enable_if<N >= 1, T> r() const { return _c[0]; }
    std::enable_if<N >= 1, T &> r() { return _c[0]; }
    std::enable_if<N >= 2, T> g() const { return _c[1]; }
    std::enable_if<N >= 2, T &> g() { return _c[1]; }
    std::enable_if<N >= 3, T> b() const { return _c[2]; }
    std::enable_if<N >= 3, T &> b() { return _c[2]; }
    std::enable_if<N >= 4, T> a() const { return _c[3]; }
    std::enable_if<N >= 4, T &> a() { return _c[3]; }

    // XYZW accessors
    std::enable_if<N >= 1, T> x() const { return _c[0]; }
    std::enable_if<N >= 1, T &> x() { return _c[0]; }
    std::enable_if<N >= 2, T> y() const { return _c[1]; }
    std::enable_if<N >= 2, T &> y() { return _c[1]; }
    std::enable_if<N >= 3, T> z() const { return _c[2]; }
    std::enable_if<N >= 3, T &> z() { return _c[2]; }
    std::enable_if<N >= 4, T> w() const { return _c[3]; }
    std::enable_if<N >= 4, T &> w() { return _c[3]; }
    // endregion

    // region simple operators
    friend Vec operator+(const Vec &lhs, const Vec &rhs) { return map(lhs, rhs, std::plus()); }
    friend Vec operator-(const Vec &lhs, const Vec &rhs) { return map(lhs, rhs, std::minus()); }
    friend Vec operator*(const Vec &lhs, const Vec &rhs) { return map(lhs, rhs, std::multiplies()); }
    friend Vec operator/(const Vec &lhs, const Vec &rhs) { return map(lhs, rhs, std::divides()); }

    friend Vec operator+(const Vec &lhs, const T &rhs) { return map(lhs, rhs, std::plus()); }
    friend Vec operator-(const Vec &lhs, const T &rhs) { return map(lhs, rhs, std::minus()); }
    friend Vec operator*(const Vec &lhs, const T &rhs) { return map(lhs, rhs, std::multiplies()); }
    friend Vec operator/(const Vec &lhs, const T &rhs) { return map(lhs, rhs, std::divides()); }

    friend Vec &operator+=(Vec &lhs, const Vec &rhs) { return lhs = lhs + rhs; }
    friend Vec &operator-=(Vec &lhs, const Vec &rhs) { return lhs = lhs - rhs; }
    friend Vec &operator*=(Vec &lhs, const Vec &rhs) { return lhs = lhs * rhs; }
    friend Vec &operator/=(Vec &lhs, const Vec &rhs) { return lhs = lhs / rhs; }

    friend Vec &operator+=(Vec &lhs, const T &rhs) { return lhs = lhs + rhs; }
    friend Vec &operator-=(Vec &lhs, const T &rhs) { return lhs = lhs - rhs; }
    friend Vec &operator*=(Vec &lhs, const T &rhs) { return lhs = lhs * rhs; }
    friend Vec &operator/=(Vec &lhs, const T &rhs) { return lhs = lhs / rhs; }

    bool operator==(const Vec &rhs) const { return _c == rhs._c; };
    bool operator!=(const Vec &rhs) const { return _c != rhs._c; };
    // endregion

    template <typename U> void write(U *ptr) const {
        if constexpr (std::is_same_v<T, U>) {
            std::memcpy(ptr, _c.begin(), N * sizeof(T));
        } else {
            for (unsigned i = 0; i < N; i++) { ptr[i] = static_cast<U>(_c[i]); }
        }
    }

    template <typename P = T, typename W = size_t>
        requires std::is_unsigned_v<P> && std::is_integral_v<T>
    P pack(const Vec<W, N> &widths) const {
        assert((sizeof(P) * 8) >= (size_t)std::accumulate(widths.begin(), widths.end(), 0));

        P packed = 0;

        for (unsigned i = 0; i < N; i++) {
            T val = at(i);
            if constexpr (std::is_signed_v<T>) { val &= ((1 << widths[i]) - 1); }  // mask out upper bits of signed vals

            assert(val < (1 << widths[i]));

            packed = (packed << widths[i]) | val;
        }
        return packed;
    }

    T sum() const { return std::accumulate(begin(), end(), T{0}); }

    T dot(const Vec &rhs) const {
        Vec product = (*this) * rhs;
        return product.sum();
    }

    T sqr_mag() const { return this->dot(*this); }

    Vec abs() const {
        return map(*this, [](T val) { return quicktex::abs(val); });
    }

    Vec clamp(const float &low, const float &high) {
        return map(*this, [&low, &high](T val) { return quicktex::clamp(val, low, high); });
    }

    Vec clamp(const Vec &low, const Vec &high) {
        Vec r;
        for (unsigned i = 0; i < N; i++) { r[i] = quicktex::clamp(at(i), low[i], high[i]); }
        return r;
    }

   protected:
    std::array<T, N> _c;  // internal array of components

    template <typename Op> static inline Vec map(const Vec &lhs, Op f) {
        Vec r;
        for (unsigned i = 0; i < N; i++) { r[i] = f(lhs[i]); }
        return r;
    }

    template <typename Op> static inline Vec map(const Vec &lhs, const T &rhs, Op f) {
        Vec r;
        for (unsigned i = 0; i < N; i++) { r[i] = f(lhs[i], rhs); }
        return r;
    }

    template <typename Op> static inline Vec map(const Vec &lhs, const Vec &rhs, Op f) {
        Vec r;
        for (unsigned i = 0; i < N; i++) { r[i] = f(lhs[i], rhs[i]); }
        return r;
    }
};

template <typename T, size_t N, typename A = xsimd::default_arch> class BatchVec : Vec<xsimd::batch<T, A>, N> {
    template <typename M = xsimd::unaligned_mode> void store(std::array<T *, N> mem_rows, M) const {
        for (unsigned i = 0; i < N; i++) { this->_c[i].store(mem_rows[i], M{}); }
    }

    template <typename M = xsimd::unaligned_mode> static Vec<T, N> load(std::array<T *, N> mem_rows, M) {
        BatchVec<T, N, A> val;
        for (unsigned i = 0; i < N; i++) { val[i] = xsimd::load<A, T>(mem_rows[i], M{}); }
        return val;
    }
};

}  // namespace quicktex