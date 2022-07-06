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

namespace quicktex {

// std::ranges is not usable by default in libc++ 13
template <class T>
concept range = requires(T &t) {
                    t.begin();
                    t.end();
                };

using std::size;
template <range T> constexpr auto size(const T &range) { return std::distance(range.begin(), range.end()); }

template <class T>
concept sized_range = range<T> && requires(T &t) { size(t); };

template <class R> using iterator_t = decltype(std::declval<R &>().begin());
template <class R> using sentinel_t = decltype(std::declval<R &>().end());
template <class R> using range_size_t = decltype(size(std::declval<R &>()));
template <class R> using range_difference_t = std::iter_difference_t<iterator_t<R>>;
template <class R> using range_value_t = std::iter_value_t<iterator_t<R>>;
template <class R> using range_reference_t = std::iter_reference_t<iterator_t<R>>;
template <class R> using range_rvalue_reference_t = std::iter_rvalue_reference_t<iterator_t<R>>;

template <class R>
concept input_range = range<R> && std::input_iterator<iterator_t<R>>;

template <class R, typename T>
concept output_range = range<R> && (std::output_iterator<iterator_t<R>, T>);

template <class R>
concept forward_range = range<R> && std::forward_iterator<iterator_t<R>>;

template <class R>
concept bidirectional_range = range<R> && std::bidirectional_iterator<iterator_t<R>>;

template <class R>
concept random_access_range = range<R> && std::random_access_iterator<iterator_t<R>>;

template <class R>
concept contiguous_range = range<R> && std::contiguous_iterator<iterator_t<R>>;

}  // namespace quicktex