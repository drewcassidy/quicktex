/*  Python-rgbcx Texture Compression Library
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

#include <array>

namespace rgbcx {
//
//class SelectorHistogram<size_t Size, size_t Max> {
//   public:
//    std::array<uint8_t, Size> histogram;
//
//    bool operator==(const SelectorHistogram<N> &other) const {
//        for (unsigned i = 0; i < Size; i++) {
//            if (histogram[i] != other.histogram[i]) return false;
//        }
//        return true;
//    }
//
//    bool AnyMax() cost {
//        for (unsigned i = 0; i < Size; i++) {
//            if (histogram[i] == Max) return true;
//        }
//        return false;
//    }
//};

}  // namespace rgbcx