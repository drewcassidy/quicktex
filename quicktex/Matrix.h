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

template <typename T, size_t N, size_t M> class Matrix;

template <typename T, size_t M> using Vec = Matrix<T, 1, M>;

// region helper concepts
template <typename L, typename R, typename Op>
concept operable = requires(L &l, R &r, Op &op) { op(l, r); };

template <typename V>
concept is_matrix = requires(V &v) {
                        V::width();
                        V::height();
                        V::value_type;
                    } && std::same_as < Matrix<typename V::value_type, V::width(), V::height()>,
std::remove_cvref_t < V >> ;

template <typename V> struct vector_stats {
    static constexpr size_t width = 1;
    static constexpr size_t height = 1;
    static constexpr size_t dims = 0;
};

template <typename V>
    requires is_matrix<V>
struct vector_stats<V> {
    static constexpr size_t width = V::width;
    static constexpr size_t height = V::height;
    static constexpr size_t dims = V::dims;
};

template <typename V> constexpr size_t vector_width = vector_stats<V>::width;
template <typename V> constexpr size_t vector_height = vector_stats<V>::height;
template <typename V> constexpr size_t vector_dims = vector_stats<V>::dims;

// endregion

template <typename T, size_t N> class VecBase {
   public:
    const T &operator[](size_t index) const { return _c[index]; }
    T &operator[](size_t index) { return _c[index]; }

    const T &at(size_t index) const { return _c.at(index); }
    T &at(size_t index) { return _c.at(index); }

    auto begin() { return _c.begin(); }
    auto begin() const { return _c.begin(); }
    auto end() { return _c.end(); }
    auto end() const { return _c.end(); }

   private:
    std::array<T, N> _c;
};

template <typename T, size_t N, size_t M> using matrix_row_type = std::conditional_t<N <= 1, T, Vec<T, N>>;
template <typename T, size_t N, size_t M> using matrix_column_type = std::conditional_t<M <= 1, T, Vec<T, M>>;

/**
 * A matrix of values that can be operated on
 * @tparam T Scalar type
 * @tparam N Width of the matrix
 * @tparam M Height of the matrix
 */
template <typename T, size_t N, size_t M>
class Matrix : public VecBase<std::conditional_t<N == 1, T, VecBase<T, N>>, M> {
   public:
    using base = VecBase<std::conditional_t<N == 1, T, VecBase<T, N>>, M>;

    using value_type = T;
    using row_type = matrix_row_type<T, N, M>;
    using column_type = matrix_column_type<T, N, M>;

    using base::base;
    using base::begin;
    using base::end;
    using base::operator[];

    // region constructors
    /**
     * Create a vector from an intializer list
     * @param il values to populate with
     */
    Matrix(std::initializer_list<row_type> il) {
        assert(il.size() == M);  // ensure il is of the right size
        std::copy_n(il.begin(), M, this->begin());
    }

    /**
     * Create a vector from a scalar value
     * @param scalar value to populate with
     */
    Matrix(const T &scalar) { std::fill(this->begin(), this->end(), scalar); }

    /**
     * Create a vector from an iterator
     * @tparam II input iterator type
     * @param input_iterator iterator to copy from
     */
    template <typename II>
    Matrix(const II input_iterator)
        requires std::input_iterator<II> && std::convertible_to<std::iter_value_t<II>,
                                                                row_type> {
        std::copy_n(input_iterator, M, this->begin());
    }

    /**
     * Create a vector from a range type
     * @tparam R Range type
     * @param input_range Range to copy from
     */
    template <typename R>
    Matrix(const R &input_range)
        requires range<R> && std::convertible_to<typename R::value_type, row_type>
    : Matrix(input_range.begin()) {
        assert(std::distance(input_range.begin(), input_range.end()) == M);
    }
    // endregion

    // region iterators and accessors
    static constexpr size_t size() { return M; }
    static constexpr size_t width = N;
    static constexpr size_t height = M;
    static constexpr size_t dims = ((width > 1) ? 1 : 0) + ((height > 1) ? 1 : 0);

    auto row_begin() { return this->begin(); }
    auto row_begin() const { return this->begin(); }

    auto row_end() { return this->end(); }
    auto row_end() const { return this->end(); }

    auto column_begin() const { return column_iterator(this, 0); }
    auto column_end() const { return column_iterator(this, N); }

    auto all_begin() const { return linear_iterator<const Matrix>(this, 0); }
    auto all_begin() { return linear_iterator<Matrix>(this, 0); }

    auto all_end() const { return linear_iterator<const Matrix>(this, N * M); }
    auto all_end() { return linear_iterator<Matrix>(this, N * M); }

    const row_type &get_row(size_t y) const { return this->at(y); }

    template <typename R> void set_row(size_t y, const R &value) { this->at(y) = value; }

    template <typename S = T> column_type get_column(size_t n) const {
        column_type ret;
        for (unsigned m = 0; m < M; m++) { ret[m] = element(m, n); }
        return ret;
    }

    void set_column(size_t n, const column_type &value) {
        column_type ret;
        for (unsigned m = 0; m < M; m++) { element(m, n) = value[m]; }
        return ret;
    }

    // n/m accessors
    const T &element(size_t m, size_t n) const {
        assert(n < N);
        assert(m < M);

        if constexpr (N == 1) {
            return this->at(m);
        } else {
            return this->at(m)[n];
        }
    }

    T &element(size_t n, size_t m) { return const_cast<T &>(static_cast<const Matrix &>(*this).element(n, m)); }

    // linear accessors
    const T &element(size_t i) const { return element(i / N, i % N); }
    T &element(size_t i) { return element(i / N, i % N); }

    // RGBA accessors
    const T &r() const { return this->at(0); }
    T &r() { return this->at(0); }
    template <typename S = T> std::enable_if_t<M >= 2, const S &> g() const { return this->at(1); }
    template <typename S = T> std::enable_if_t<M >= 2, S &> g() { return this->at(1); }
    template <typename S = T> std::enable_if_t<M >= 3, const S &> b() const { return this->at(2); }
    template <typename S = T> std::enable_if_t<M >= 3, S &> b() { return this->at(2); }
    template <typename S = T> std::enable_if_t<M >= 4, const S &> a() const { return this->at(3); }
    template <typename S = T> std::enable_if_t<M >= 4, S &> a() { return this->at(3); }

    // XYZW accessors
    const T &x() const { return this->at(0); }
    T &x() { return this->at(0); }
    template <typename S = T> std::enable_if_t<M >= 2, const S &> y() const { return this->at(1); }
    template <typename S = T> std::enable_if_t<M >= 2, S &> y() { return this->at(1); }
    template <typename S = T> std::enable_if_t<M >= 3, const S &> z() const { return this->at(2); }
    template <typename S = T> std::enable_if_t<M >= 3, S &> z() { return this->at(2); }
    template <typename S = T> std::enable_if_t<M >= 4, const S &> w() const { return this->at(3); }
    template <typename S = T> std::enable_if_t<M >= 4, S &> w() { return this->at(3); }
    // endregion

    template <typename R>
        requires std::equality_comparable_with<T, R> bool
    operator==(const Matrix<R, N, M> &rhs) const {
        return size() == rhs.size() && std::equal(this->begin(), this->end(), rhs.begin());
    };

    // unary vector negation
    template <typename S = T>
        requires(!std::unsigned_integral<T>) && requires(T &t) { -t; }
    Matrix operator-() const {
        return map(*this, std::negate());
    };

    // add vectors
    template <typename R>
        requires operable<R, T, std::plus<>>
    Matrix operator+(const Matrix<R, N, M> &rhs) const {
        return map(*this, rhs, std::plus());
    };

    // subtract vectors
    template <typename R>
        requires operable<R, T, std::minus<>>
    Matrix operator-(const Matrix<R, N, M> &rhs) const {
        // we can't just add the negation because that's invalid for unsigned types
        return map(*this, rhs, std::minus());
    };

    // multiply matrix with a matrix or column vector
    template <typename R, size_t NN>
        requires(NN == 1 || NN == N) && operable<R, T, std::multiplies<>>
    Matrix operator*(const Matrix<R, NN, M> &rhs) const {
        return map(*this, rhs, std::multiplies());
    };

    // multiply matrix with a scalar
    template <typename R>
        requires operable<R, T, std::multiplies<>>
    Matrix operator*(const R &rhs) const {
        return map(*this, rhs, std::multiplies());
    };

    // divides a matrix by a matrix or column vector
    template <typename R, size_t NN>
        requires(NN == 1 || NN == N) && operable<R, T, std::divides<>>
    Matrix operator/(const Matrix<R, NN, M> &rhs) const {
        return map(*this, rhs, std::divides());
    };

    // divides a matrix by a scalar
    template <typename R>
        requires operable<R, T, std::divides<>>
    Matrix operator/(const R &rhs) const {
        return map(*this, rhs, std::divides());
    };

    // add-assigns a matrix with a matrix
    template <typename R>
        requires operable<Matrix, R, std::plus<>>
    Matrix &operator+=(const R &rhs) {
        return *this = *this + rhs;
    }

    // subtract-assigns a matrix with a matrix
    template <typename R>
        requires operable<Matrix, R, std::minus<>>
    Matrix &operator-=(const R &rhs) {
        return *this = *this - rhs;
    }

    // multiply-assigns a matrix with a matrix, column vector, or a scalar
    template <typename R>
        requires operable<Matrix, R, std::multiplies<>>
    Matrix &operator*=(const R &rhs) {
        return *this = *this * rhs;
    }

    // divide-assigns a matrix by a matrix, column vector, or a scalar
    template <typename R>
        requires operable<Matrix, R, std::divides<>>
    Matrix &operator/=(const R &rhs) {
        return *this = *this / rhs;
    }

    // decay a 1x1 matrix to a scalar on demand
    template <typename S = T>
        requires(N == 1 && M == 1)
    operator S &() {
        return this->at(0);
    }
    template <typename S = T>
        requires(N == 1 && M == 1)
    operator const S &() const {
        return this->at(0);
    }

    // sum up all columns
    column_type hsum() const { return std::accumulate(column_begin(), column_end(), column_type{}); }

    // sum up all rows
    row_type vsum() const { return std::accumulate(row_begin(), row_end(), row_type{}); }

    // sum up all values
    T sum() const { return std::accumulate(all_begin(), all_end(), T{}); }

    template <typename R, size_t P>
        requires operable<R, T, std::multiplies<>>
    Matrix<T, P, M> mult(const Matrix<R, P, N> &rhs) {
        auto rt = rhs.transpose();
        Matrix<T, P, M> res(0);
        for (unsigned i = 0; i < P; i++) {
            // for each column of the RHS/Result
            for (unsigned j = 0; j < M; j++) {
                // for each row of the LHS/Result
                res.element(i, j) = get_row(i).dot(rt.get_row(j));
            }
        }
        return res;
    }

    Matrix<T, M, N> transpose() {
        Matrix<T, M, N> res;
        for (unsigned m = 0; m < M; m++) { res.set_column(m, get_row(m)); }
        return res;
    }

    // dot product of two compatible matrices
    template <typename R>
        requires(N == 1) && operable<T, R, std::multiplies<>> && operable<T, T, std::plus<>>
    row_type dot(const Matrix<R, N, M> &rhs) const {
        // technically this is Lt * R, but the vsum method is probably faster/more readable
        // than allocationg a new transpose matrix
        Matrix product = *this * rhs;
        return product.vsum();
    }

    row_type sqr_mag() const { return dot(*this); }

    Matrix abs() const {
        Matrix ret;
        for (unsigned i = 0; i < N * M; i++) { ret.element(i) = quicktex::abs(element(i)); }
        return ret;
    }

    Matrix clamp(T low, T high) {
        Matrix ret;
        for (unsigned i = 0; i < N * M; i++) { ret.element(i) = quicktex::clamp(element(i), low, high); }
        return ret;
    }

    Matrix clamp(const Matrix &low, const Matrix &high) {
        Matrix ret;
        for (unsigned i = 0; i < N * M; i++) {
            ret.element(i) = quicktex::clamp(element(i), low.element(i), high.element(i));
        }
        return ret;
    }

   protected:
    template <typename Op> static inline Matrix map(Matrix &lhs, Op f) {
        Matrix ret;
        for (unsigned i = 0; i < lhs.size(); i++) { ret[i] = f(lhs[i]); }
        return ret;
    }

    template <typename Op, typename R>
        requires operable<R, T, Op>
    static inline Matrix map(const Matrix &lhs, const R &rhs, Op f) {
        Matrix r;
        for (unsigned i = 0; i < lhs.size(); i++) { r[i] = f(lhs[i], rhs); }
        return r;
    }

    template <typename Op, typename R>
        requires operable<R, T, Op>
    static inline Matrix map(const Matrix &lhs, const Matrix<R, N, M> &rhs, Op f) {
        Matrix r;
        for (unsigned i = 0; i < lhs.size(); i++) { r[i] = f(lhs[i], rhs[i]); }
        return r;
    }

    class column_iterator : public index_iterator_base<column_iterator> {
       public:
        using value_type = column_type;
        using base = index_iterator_base<column_iterator>;

        column_iterator(const Matrix *matrix = nullptr, size_t index = 0) : base(index), _matrix(matrix){};

        column_type operator*() const { return _matrix->get_column(this->_index); }
        const column_type *operator->() const { &(_matrix->get_column(this->_index)); }

        friend bool operator==(const column_iterator &lhs, const column_iterator &rhs) {
            return (lhs._matrix == rhs._matrix) && (lhs._index == rhs._index);
        }

       private:
        const Matrix *_matrix;
    };

    template <typename V> class linear_iterator : public index_iterator_base<linear_iterator<V>> {
       public:
        using value_type = column_type;
        using base = index_iterator_base<linear_iterator<V>>;

        linear_iterator(V *matrix = nullptr, size_t index = 0) : base(index), _matrix(matrix){};

        auto operator*() const { return _matrix->element(this->_index); }
        auto *operator->() const { return &(_matrix->element(this->_index)); }

        friend bool operator==(const linear_iterator &lhs, const linear_iterator &rhs) {
            return (lhs._matrix == rhs._matrix) && (lhs._index == rhs._index);
        }

       private:
        V *_matrix;
    };
};

template <typename T, size_t M, typename A = xsimd::default_arch> class BatchVec : Vec<xsimd::batch<T, A>, M> {
    template <size_t N, typename U = xsimd::unaligned_mode>
    static BatchVec load_columns(const Matrix<T, N, M> &matrix, size_t column) {
        const size_t batch_size = xsimd::batch<T, A>::size;
        assert(column + batch_size <= N);

        BatchVec ret;
        for (unsigned i = 0; i < M; i++) { ret[i] = xsimd::load<A, T>(&(matrix[column][i]), U{}); }
        return ret;
    }

    template <typename U = xsimd::unaligned_mode, typename V, size_t N>
    void store_columns(Matrix<T, N, M> &matrix, size_t column) {
        const size_t batch_size = xsimd::batch<T, A>::size;
        assert(column + batch_size <= N);

        for (unsigned i = 0; i < M; i++) { this->at(i).store((&(matrix[column][i]), U{})); }
    }
};

}  // namespace quicktex