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

#include <algorithm>
#include <cstdint>
#include <numeric>
#include <xsimd/xsimd.hpp>

#include "util/math.h"
#include "util/types.h"

namespace quicktex {

#pragma pack(push, 1)
template <typename T, size_t N> class Vec {
   public:
    typedef T value_type;

    // region constructors
    /**
     * Create a vector from an intializer list
     * @param vals values to populate with
     */
    Vec(std::initializer_list<T> vals) { std::copy(vals.begin(), vals.end(), begin()); }

    /**
     * Create a vector from a scalar value
     * @param scalar value to populate with
     */
    Vec(const T &scalar = 0) { std::fill(begin(), end(), scalar); }

    /**
     * Create a vector from another vector of the same size and another type
     * @tparam S Source vector type
     * @param rvalue Source vector to copy from
     */
    template <typename S> Vec(std::enable_if_t<std::is_convertible_v<S, T>, const Vec<S, N>> &rvalue) {
        static_assert(sizeof(Vec) == N * sizeof(T));
        for (unsigned i = 0; i < N; i++) { at(i) = static_cast<T>(rvalue[i]); }
    }

    /**
     * Create a vector from an iterator
     * @tparam II input iterator type
     * @param input_iterator iterator to copy from
     */
    template <typename II> Vec(const II input_iterator) { std::copy_n(input_iterator, N, begin()); }

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

    T *begin() { return &_c[0]; }
    T *end() { return &_c[N]; }
    const T *begin() const { return &_c[0]; }
    const T *end() const { return &_c[N]; }
    size_t size() const { return N; }

    // endregion

    // region accessor shortcuts
    // RGBA accessors
    T r() const { return _c[0]; }
    T &r() { return _c[0]; }
    template <typename S = T> std::enable_if_t<N >= 2, S> g() const { return _c[1]; }
    template <typename S = T> std::enable_if_t<N >= 2, S &> g() { return _c[1]; }
    template <typename S = T> std::enable_if_t<N >= 3, S> b() const { return _c[2]; }
    template <typename S = T> std::enable_if_t<N >= 3, S &> b() { return _c[2]; }
    template <typename S = T> std::enable_if_t<N >= 4, S> a() const { return _c[3]; }
    template <typename S = T> std::enable_if_t<N >= 4, S &> a() { return _c[3]; }

    // XYZW accessors
    T x() const { return _c[0]; }
    T &x() { return _c[0]; }
    template <typename S = T> std::enable_if_t<N >= 2, S> y() const { return _c[1]; }
    template <typename S = T> std::enable_if_t<N >= 2, S &> y() { return _c[1]; }
    template <typename S = T> std::enable_if_t<N >= 3, S> z() const { return _c[2]; }
    template <typename S = T> std::enable_if_t<N >= 3, S &> z() { return _c[2]; }
    template <typename S = T> std::enable_if_t<N >= 4, S> w() const { return _c[3]; }
    template <typename S = T> std::enable_if_t<N >= 4, S &> w() { return _c[3]; }
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

    bool operator==(const Vec &rhs) const { return std::equal(begin(), end(), rhs.begin()); };
    bool operator!=(const Vec &rhs) const { return !(*this == rhs); };
    // endregion

    template <typename OI> void copy(OI output_iterator) const { std::copy(begin(), end(), output_iterator); }

    T sum() const { return std::accumulate(begin(), end(), T{0}); }

    T dot(const Vec &rhs) const {
        Vec product = (*this) * rhs;
        return product.sum();
    }

    T sqr_mag() const { return this->dot(*this); }

    Vec abs() const {
        return map(*this, [](T val) { return quicktex::abs(val); });
    }

    Vec clamp(float low, float high) {
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
#pragma pack(pop)

}  // namespace quicktex