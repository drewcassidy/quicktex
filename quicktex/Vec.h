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
#include "util/ranges.h"

namespace quicktex {

template <typename V>
concept vector_like = subscriptable_range<V> && requires { V::size(); };

template <typename V> constexpr size_t vector_dims = vector_like<V> ? 1 + vector_dims<range_value_t<V>> : 0;

template <typename T, size_t N> class Vec;

namespace detail {
template <typename T> struct _vector_stype { using type = T; };

template <typename T>
    requires vector_like<T>
struct _vector_stype<T> {
    using type = range_value_t<T>;
};

template <typename T, size_t N = 1, size_t M = 1> struct _make_matrix { using type = T; };

template <typename T, size_t N, size_t M>
    requires(N > 1 && M == 1)
struct _make_matrix<T, N, M> {
    using type = Vec<T, N>;
};

template <typename T, size_t N, size_t M>
    requires(N > 1 && M > 1)
struct _make_matrix<T, N, M> {
    using type = Vec<Vec<T, N>, M>;
};

template <typename T> struct _vector_width { static constexpr size_t width = 1; };
template <typename T>
    requires vector_like<T>
struct _vector_width<T> {
    static constexpr size_t width = T::size();
};

template <typename T> struct _vector_height { static constexpr size_t height = 1; };
template <typename T>
    requires vector_like<T>
struct _vector_height<T> {
    static constexpr size_t height = _vector_width<range_value_t<T>>::width;
};

}  // namespace detail

template <typename T> using vector_stype = typename detail::_vector_stype<T>::type;
template <typename T, size_t N = 1, size_t M = 1> using make_matrix = typename detail::_make_matrix<T, N, M>::type;
template <typename T> constexpr size_t vector_width = detail::_vector_width<T>::width;
template <typename T> constexpr size_t vector_height = detail::_vector_height<T>::height;

template <typename L, typename R, typename Op>
concept operable_VV = vector_like<L> && vector_like<R> && (vector_dims<L> == vector_dims<R>) &&
                      (vector_width<L> == vector_width<R>) &&
                      requires(range_value_t<L> &l, range_value_t<R> &r, Op &op) { op(l, r); };

template <typename L, typename R, typename Op>
concept operable_Vs = vector_like<L> && (!vector_like<R>) && requires(range_value_t<L> &l, R &r, Op &op) { op(l, r); };

template <typename T, size_t N, typename D> class VecBase {
   public:
    // region constructors
    /**
     * Create a vector from an intializer list
     * @param il values to populate with
     */
    VecBase(std::initializer_list<T> il) {
        assert(il.size() == N);  // ensure il is of the right size
        std::copy_n(il.begin(), N, begin());
    }

    /**
     * Create a vector from a scalar value
     * @param scalar value to populate with
     */
    VecBase(const T &scalar) { std::fill(begin(), end(), scalar); }

    /**
     * Create a vector from an iterator
     * @tparam II input iterator type
     * @param input_iterator iterator to copy from
     */
    template <typename II>
    VecBase(const II input_iterator)
        requires std::input_iterator<II> && std::convertible_to<std::iter_value_t<II>,
                                                                T> {
        std::copy_n(input_iterator, N, begin());
    }

    /**
     * Create a vector from a range type
     * @tparam R Range type
     * @param input_range Range to copy from
     */
    template <typename R>
    VecBase(const R &input_range)
        requires range<R> && std::convertible_to<typename R::value_type, T>
    : VecBase(input_range.begin()) {
        assert(std::distance(input_range.begin(), input_range.end()) == N);
    }
    // endregion

    // region iterators and accessors
    static constexpr size_t size() { return N; }
    inline auto begin() { return this->_derived()._begin(); }
    inline auto begin() const { return this->_derived()._begin(); }
    inline auto end() { return this->_derived()._end(); }
    inline auto end() const { return this->_derived()._end(); }

    inline T &at(size_t i) {
        assert(i < N);
        return this->_derived()._at(i);
    }
    inline const T &at(size_t i) const {
        assert(i < N);
        return this->_derived()._at(i);
    }

    const T &operator[](size_t i) const { return at(i); }
    T &operator[](size_t i) { return at(i); }

    const T &get_row(size_t y) const { return at(y); }

    template <typename R>
        requires vector_like<R> && (R::size() == N)
    void set_row(size_t y, const R &value) {
        at(y) = value;
    }

    const auto &get_column(size_t x) const {
        make_matrix<vector_stype<D>, vector_height<D>> ret;
        if constexpr (vector_height<D> == 1) {
            return at(x);
        }
        for (unsigned y = 0; y < vector_height<D>; y++) { ret[y] = at(x)[y]; }
        return ret;
    }

    template <typename R>
        requires vector_like<R> && (R::size() == vector_height<D>)
    void set_column(size_t x, const R &value) {
        for (unsigned y = 0; y < vector_height<D>; y++) { at(x)[y] = value[y]; }
    }

    // RGBA accessors
    const T &r() const { return at(0); }
    T &r() { return at(0); }
    template <typename S = T> std::enable_if_t<N >= 2, const S &> g() const { return at(1); }
    template <typename S = T> std::enable_if_t<N >= 2, S &> g() { return at(1); }
    template <typename S = T> std::enable_if_t<N >= 3, const S &> b() const { return at(2); }
    template <typename S = T> std::enable_if_t<N >= 3, S &> b() { return at(2); }
    template <typename S = T> std::enable_if_t<N >= 4, const S &> a() const { return at(3); }
    template <typename S = T> std::enable_if_t<N >= 4, S &> a() { return at(3); }

    // XYZW accessors
    const T &x() const { return at(0); }
    T &x() { return at(0); }
    template <typename S = T> std::enable_if_t<N >= 2, const S &> y() const { return at(1); }
    template <typename S = T> std::enable_if_t<N >= 2, S &> y() { return at(1); }
    template <typename S = T> std::enable_if_t<N >= 3, const S &> z() const { return at(2); }
    template <typename S = T> std::enable_if_t<N >= 3, S &> z() { return at(2); }
    template <typename S = T> std::enable_if_t<N >= 4, const S &> w() const { return at(3); }
    template <typename S = T> std::enable_if_t<N >= 4, S &> w() { return at(3); }
    // endregion

    //    template <typename R>
    //        requires sized_range<R> bool
    bool operator==(const VecBase &rhs) const {
        return size() == rhs.size() && std::equal(begin(), end(), rhs.begin());
    };

    // unary vector negation
    template <typename S = T>
        requires(!std::unsigned_integral<T>) && requires(T &t) { -t; }
    D operator-() const {
        return map(_derived(), std::negate());
    };

    // add vectors
    template <typename R>
        requires operable_VV<D, R, std::plus<>>
    D operator+(const R &rhs) const {
        return map(_derived(), rhs, std::plus());
    };

    // subtract vectors
    template <typename R>
        requires operable_VV<D, R, std::minus<>>
    D operator-(const R &rhs) const {
        // we can't just add the negation because that's invalid for unsigned types
        return map(_derived(), rhs, std::minus());
    };

    // multiply vector with a vector or scalar
    template <typename R>
        requires operable_VV<D, R, std::multiplies<>> || operable_Vs<D, R, std::multiplies<>>
    D operator*(const R &rhs) const {
        return map(_derived(), rhs, std::multiplies());
    };

    // multiply a scalar by a vector
    template <typename L>
        requires operable_Vs<D, L, std::multiplies<>>
    friend D operator*(const L &lhs, const D &rhs) {
        return rhs * lhs;
    }

    // divides vector with a vector or scalar
    template <typename R>
        requires operable_VV<D, R, std::divides<>> || operable_Vs<D, R, std::divides<>>
    D operator/(const R &rhs) const {
        return map(_derived(), rhs, std::divides());
    };

    template <typename R>
        requires operable_VV<D, R, std::plus<>>
    D &operator+=(const R &rhs) {
        return _derived() = _derived() + rhs;
    }

    template <typename R>
        requires operable_VV<D, R, std::minus<>>
    D &operator-=(const R &rhs) {
        return _derived() = _derived() - rhs;
    }

    template <typename R>
        requires operable_VV<D, R, std::multiplies<>> || operable_Vs<D, R, std::multiplies<>>
    D &operator*=(const R &rhs) {
        return _derived() = _derived() * rhs;
    }

    template <typename R>
        requires operable_VV<D, R, std::divides<>> || operable_Vs<D, R, std::divides<>>
    D &operator/=(const R &rhs) {
        return _derived() = _derived() / rhs;
    }

    auto hsum() const {
        make_matrix<vector_stype<D>, vector_height<D>> acc;
        for (unsigned n = 0; n < N; n++) { acc += get_column(n); }
        return acc;
    }

    template <typename V>
        requires vector_like<V> && (vector_dims<V> == vector_dims<D>) && (vector_width<V> == vector_width<D>) &&
                 (vector_height<V> == vector_height<D>)
    auto dot(const V &rhs) const {
        auto product = _derived() * rhs;
        return product.hsum();
    }

    auto sqr_mag() const { return this->dot(_derived()); }

    D abs() const {
        return map(_derived(), [](T val) { return quicktex::abs(val); });
    }

    D clamp(T low, T high) {
        return map(_derived(), [&low, &high](T val) { return quicktex::clamp(val, low, high); });
    }

    template <typename L, typename H>
        requires vector_like<L> && vector_like<H> && (L::size() == H::size())
    D clamp(const L &low, const H &high) {
        D r;
        for (unsigned i = 0; i < N; i++) { r[i] = quicktex::clamp(at(i), low[i], high[i]); }
        return r;
    }

   protected:
    template <typename Op, typename L>
        requires subscriptable_range<L>
    static inline D map(const L &lhs, Op f) {
        D r;
        for (unsigned i = 0; i < lhs.size(); i++) { r[i] = f(lhs[i]); }
        return r;
    }

    template <typename Op, typename L, typename R>
        requires operable_Vs<L, R, Op>
    static inline D map(const L &lhs, const R &rhs, Op f) {
        D r;
        for (unsigned i = 0; i < lhs.size(); i++) { r[i] = f(lhs[i], rhs); }
        return r;
    }

    template <typename Op, typename L, typename R>
        requires operable_VV<L, R, Op>
    static inline D map(const L &lhs, const R &rhs, Op f) {
        D r;
        for (unsigned i = 0; i < N; i++) { r[i] = f(lhs[i], rhs[i]); }
        return r;
    }

   private:
    // error guard constructor prevents incorrect CRTP usage
    VecBase() {
        static_assert(vector_like<D>);  // obviousy a vector is vector-like
    };
    friend D;

    D &_derived() { return static_cast<D &>(*this); }
    D const &_derived() const { return static_cast<D const &>(*this); }
};

#pragma pack(push, 1)
template <typename T, size_t N> class Vec : public VecBase<T, N, Vec<T, N>> {
   public:
    typedef T value_type;
    typedef VecBase<T, N, Vec<T, N>> base;

    friend base;
    using base::base;  // import base constructors

    Vec() : _c() {}  // default constructor

   protected:
    const T &_at(size_t i) const { return _c[i]; }

    T &_at(size_t i) { return _c[i]; }

    auto _begin() { return _c.begin(); }
    auto _begin() const { return _c.begin(); }

    auto _end() { return _c.end(); }
    auto _end() const { return _c.end(); }

   private:
    std::array<T, N> _c;  // internal array of components
};

template <typename T, size_t M, typename A = xsimd::default_arch> class BatchVec : Vec<xsimd::batch<T, A>, M> {
    template <size_t N, typename U = xsimd::unaligned_mode>
    static BatchVec load_columns(const make_matrix<T, N, M> &matrix, size_t column) {
        const size_t batch_size = xsimd::batch<T, A>::size;
        assert(column + batch_size <= N);

        BatchVec ret;
        for (unsigned i = 0; i < M; i++) { ret[i] = xsimd::load<A, T>(&(matrix[column][i]), U{}); }
        return ret;
    }

    template <typename U = xsimd::unaligned_mode, typename V, size_t N>
    void store_columns(make_matrix<T, N, M> &matrix, size_t column) {
        const size_t batch_size = xsimd::batch<T, A>::size;
        assert(column + batch_size <= N);

        for (unsigned i = 0; i < M; i++) { this->at(i).store((&(matrix[column][i]), U{})); }
    }
};
#pragma pack(pop)

}  // namespace quicktex