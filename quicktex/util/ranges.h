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
#include <cassert>
#include <cstdint>
#include <functional>
#include <iterator>
#include <limits>
#include <numeric>
#include <string>
#include <type_traits>
#include <vector>

#include "xsimd/xsimd.hpp"

namespace quicktex {

// std::ranges::range is currently not usable by default in libc++
template <class T>
concept range = requires(T &t) {
                    std::begin(t);
                    std::end(t);
                };

template <class T>
concept sized_range = range<T> && requires(T &t) { std::size(t); };

template <class T>
    requires range<T>
size_t distance(T range) {
    return std::distance(range.begin(), range.end());
}

template <typename T> class const_iterator {
   public:
    typedef long long difference_type;
    typedef T value_type;

    const_iterator() : _value(T{}), _index(0) {}
    const_iterator(T value, size_t index = 0) : _value(value), _index(index) {}

    const_iterator &operator++() {
        _index++;
        return *this;
    }
    const_iterator operator++(int) {
        const_iterator old = *this;
        _index++;
        return old;
    }
    const_iterator &operator--() {
        _index++;
        return *this;
    }
    const_iterator operator--(int) {
        const_iterator old = *this;
        _index++;
        return old;
    }

    T operator*() const { return _value; }

    difference_type operator-(const_iterator rhs) const { return (difference_type)_index - rhs._index; }
    const_iterator operator+(size_t rhs) const { return const_iterator(rhs + _index); }
    const_iterator operator-(size_t rhs) const { return const_iterator(rhs - _index); }

    friend bool operator==(const const_iterator &lhs, const const_iterator &rhs) {
        return (lhs._value == rhs._value) && (lhs._index == rhs._index);
    }

   private:
    T _value;
    size_t _index;
};

template <typename Seq, typename Fn> constexpr auto map(const Seq &input, Fn op) {
    using I = typename Seq::value_type;
    using O = decltype(op(I{}));
    constexpr size_t N = std::tuple_size<Seq>::value;

    std::array<O, N> output;
    for (unsigned i = 0; i < N; i++) { output[i] = op(input[i]); }
    return output;
}
}  // namespace quicktex::util