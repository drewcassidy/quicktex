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

namespace quicktex::util {
template <class> struct next_size;
template <class T> using next_size_t = typename next_size<T>::type;
template <class T> struct next_size_tag { using type = T; };

template <> struct next_size<int8_t> : next_size_tag<int16_t> {};
template <> struct next_size<int16_t> : next_size_tag<int32_t> {};
template <> struct next_size<int32_t> : next_size_tag<int64_t> {};

template <> struct next_size<uint8_t> : next_size_tag<uint16_t> {};
template <> struct next_size<uint16_t> : next_size_tag<uint32_t> {};
template <> struct next_size<uint32_t> : next_size_tag<uint64_t> {};
}  // namespace quicktex::util