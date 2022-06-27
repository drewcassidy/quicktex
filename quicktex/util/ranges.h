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
#include <array>
#include <cassert>
#include <cstdint>
#include <functional>
#include <iterator>
#include <limits>
#include <numeric>
#include <string>
#include <type_traits>
#include <vector>

namespace quicktex {

// std::ranges::range is not usable by default in libc++ 13
template <class T>
concept range = std::is_constructible_v<T> && requires(T &t) {
                                                  t.begin();
                                                  t.end();
                                              };

template <class T>
concept sized = requires(T &t) { std::size(t); };

template <class T>
concept sized_range = range<T> && sized<T>;

template <typename T>
concept const_subscriptable = requires(T &t) {
                                  t[0];
                                  std::size(t);
                              };

template <class T>
concept subscriptable = const_subscriptable<T> && requires(T &t) {
                                                      t[0] = std::declval<decltype(t[0])>();  // subscript is assignable
                                                  };

template <typename T>
concept subscriptable_range = sized_range<T> && subscriptable<T>;

template <typename T> struct range_value { using type = void; };

template <typename T>
    requires requires(T &t) { std::begin(t); }
struct range_value<T> {
    using type = std::iter_value_t<decltype((std::declval<T>()).begin())>;
};

template <typename T> using range_value_t = typename range_value<T>::type;

// some quick inline checks
static_assert(const_subscriptable<const std::array<int, 4>>);  // const array can be subscripted
static_assert(!subscriptable<const std::array<int, 4>>);       // const array cannot be assigned to
static_assert(subscriptable_range<std::array<int, 4>>);  // array is subscriptable, sized, and has begin() and end()
static_assert(sized_range<std::initializer_list<int>>);  // initializer list is a range and has size()
static_assert(std::same_as<range_value_t<int>, void>);

template <class T>
    requires range<T>
size_t distance(T range) {
    return std::distance(range.begin(), range.end());
}

template <class II>
    requires std::input_or_output_iterator<II>
class view {
   public:
    view() : _begin(), _end() {}
    view(II begin, II end) : _begin(begin), _end(end) {}

    inline size_t size() { return distance(_begin, _end); }
    inline II begin() { return _begin; }
    inline II end() { return _end; }

   private:
    II _begin;
    II _end;
};

template <typename D> class index_iterator_base {
   public:
    typedef long long difference_type;

    D &operator++() {
        _index++;
        return static_cast<D &>(*this);
    }
    D operator++(int) {
        D old = static_cast<D &>(*this);
        _index++;
        return old;
    }
    D &operator--() {
        _index++;
        return static_cast<D &>(*this);
    }
    D operator--(int) {
        D old = static_cast<D &>(*this);
        _index++;
        return old;
    }

    D operator+(difference_type rhs) const {
        D d = static_cast<const D &>(*this);
        d._index += rhs;
        return d;
    }

    D operator-(difference_type rhs) const {
        D d = static_cast<const D &>(*this);
        d._index -= rhs;
        return d;
    }

    D &operator+=(difference_type rhs) {
        *this = *this + rhs;
        return *this;
    }

    D &operator-=(difference_type rhs) {
        *this = *this - rhs;
        return *this;
    }

    difference_type operator-(const D &rhs) const { return (difference_type)_index - rhs._index; }

    friend D operator+(difference_type lhs, const D &rhs) { return rhs + lhs; }

    friend auto operator<=>(const D &lhs, const D &rhs) { return lhs._index <=> rhs._index; }

    auto &operator[](difference_type i) { return *(static_cast<D &>(*this) + i); }
    auto operator[](difference_type i) const { return *(static_cast<const D &>(*this) + i); }

   protected:
    size_t _index;

   private:
    friend D;
    index_iterator_base(size_t index = 0) : _index(index) {}
};

template <typename T> class const_iterator : public index_iterator_base<const_iterator<T>> {
   public:
    typedef index_iterator_base<const_iterator<T>> base;
    typedef long long difference_type;
    typedef T value_type;

    const_iterator() : base(0), _value(T{}) {}
    const_iterator(T value, size_t index = 0) : base(index), _value(value) {}

    T operator*() const { return _value; }
    const T *operator->() const { return &_value; }

    friend bool operator==(const const_iterator &lhs, const const_iterator &rhs) {
        return (lhs._value == rhs._value) && (lhs._index == rhs._index);
    }

   private:
    T _value;
};

// const_iterator is guaranteed to be a random access iterator. it is not writable for obvious reasons
static_assert(std::random_access_iterator<const_iterator<int>>);

template <typename R>
    requires subscriptable<R>
class index_iterator : public index_iterator_base<index_iterator<R>> {
   public:
    using base = index_iterator_base<index_iterator<R>>;
    using difference_type = long long;
    using value_type = range_value_t<R>;

    index_iterator() : base(0), _range(nullptr) {}
    index_iterator(R &range, size_t index) : base(index), _range(&range) {}

    auto operator*() const {
        // if we have the information, do a bounds check
        if constexpr (sized<R>) { assert(this->_index < std::size(*_range)); }
        return (*_range)[this->_index];
    }
    auto *operator->() const { return &((*_range)[this->_index]); }

    friend bool operator==(const index_iterator &lhs, const index_iterator &rhs) {
        return (lhs._range == rhs._range) && (lhs._index == rhs._index);
    }

   private:
    R *_range;
};

// index_iterator satisfied forward_iterator
static_assert(std::random_access_iterator<index_iterator<std::array<int, 4>>>);
}  // namespace quicktex