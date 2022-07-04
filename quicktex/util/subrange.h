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

#include <concepts>
#include <iterator>

#include "util/ranges.h"

namespace quicktex {

template <std::input_or_output_iterator I, std::sentinel_for<I> S = I> struct subrange {
   public:
    using iterator_type = I;
    using sentinel_type = S;
    using value_type = std::iter_value_t<I>;
    using reference_type = std::iter_reference_t<I>;
    using difference_type = std::iter_difference_t<I>;

    constexpr subrange(const I& b, const S& e) : _begin(b), _end(e) {}

    constexpr I begin() const { return _begin; }
    constexpr S end() const { return _end; }
    constexpr bool empty() const { return _begin == _end; }
    constexpr difference_type size() const { return std::distance(_end, _begin); }

    explicit constexpr operator bool() const { return !empty(); }

    constexpr subrange& advance(difference_type n) {
        assert(n >= 0 || std::bidirectional_iterator<I>);  // forward iterators cannot be decremented

        if (n > 0) {
            for (int i = 0; i < n && _begin != _end; i++) { _begin++; }
        } else {
            for (int i = 0; i > n && _begin != _end; i--) { _begin--; }
        }
        return *this;
    }

    constexpr subrange next(difference_type n = 1) const {
        auto tmp = *this;
        return tmp.advance(n);
    }

    template <typename _ = I>
        requires std::bidirectional_iterator<I>
    constexpr subrange prev(difference_type n = 1) const {
        return next(-n);
    }

    template <typename _ = I>
        requires std::random_access_iterator<I>
    constexpr reference_type operator[](difference_type i) {
        assert(i >= 0 && i < size());
        return _begin[i];
    }

    template <typename _ = I>
        requires std::random_access_iterator<I>
    constexpr const reference_type operator[](difference_type i) const {
        assert(i >= 0 && i < size());
        return _begin[i];
    }

    template <typename _ = I>
        requires std::contiguous_iterator<I>
    constexpr value_type* data() {
        return std::to_address(_begin);
    }
    template <typename _ = I>
        requires std::contiguous_iterator<I>
    constexpr value_type const* data() const {
        return std::to_address(_begin);
    }

   private:
    I _begin;
    S _end;
};
}  // namespace quicktex