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

namespace quicktex {

namespace detail {
template <class R> using subs_value_t = std::remove_reference_t<decltype(std::declval<R &>()[0])>;
}

template <typename D, typename T> class index_iterator_base {
   public:
    using value_type = T;
    using size_type = int;
    using difference_type = int;

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
        _index--;
        return static_cast<D &>(*this);
    }
    D operator--(int) {
        D old = static_cast<D &>(*this);
        _index--;
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

    T &operator[](difference_type i) { return *(static_cast<D &>(*this) + i); }
    T &operator[](difference_type i) const { return *(static_cast<const D &>(*this) + i); }

   protected:
    int _index;

   private:
    friend D;
    index_iterator_base(size_t index = 0) : _index(index) {}
};

template <typename R>
    requires requires(const R &r) { r[0]; }
class index_iterator : public index_iterator_base<index_iterator<R>, detail::subs_value_t<R>> {
   public:
    using base = index_iterator_base<index_iterator<R>, detail::subs_value_t<R>>;
    using typename base::difference_type;
    using typename base::size_type;
    using typename base::value_type;

    index_iterator() : base(0), _range(nullptr) {}
    index_iterator(R &range, int index) : base(index), _range(&range) {}

    value_type &operator*() const {
        assert(_range != nullptr);
        assert(this->_index >= 0);
        assert(this->_index < (size_type)_range->size());
        return (*_range)[this->_index];
    }
    value_type *operator->() const { return &(this->operator*()); }

    friend bool operator==(const index_iterator &lhs, const index_iterator &rhs) {
        return (lhs._range == rhs._range) && (lhs._index == rhs._index);
    }

   private:
    R *_range;
};

template <typename T> class const_iterator : public index_iterator_base<const_iterator<T>, const T> {
   public:
    using base = index_iterator_base<const_iterator<T>, const T>;
    using typename base::difference_type;
    using typename base::size_type;
    using typename base::value_type;

    const_iterator() : base(0), _value(T{}) {}
    const_iterator(T value, int index = 0) : base(index), _value(value) {}

    value_type &operator*() const { return _value; }
    value_type *operator->() const { return &_value; }

    friend bool operator==(const const_iterator &lhs, const const_iterator &rhs) {
        return (lhs._value == rhs._value) && (lhs._index == rhs._index);
    }

   private:
    T _value;
};

// const_iterator is guaranteed to be a random access iterator. it is not writable for obvious reasons
static_assert(std::random_access_iterator<const_iterator<int>>);

// index_iterator satisfied forward_iterator
static_assert(std::random_access_iterator<index_iterator<std::array<int, 4>>>);
}  // namespace quicktex