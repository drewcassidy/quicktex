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

#include <array>

#include "Vec.h"

namespace quicktex {
template <typename T, size_t M, size_t N> class Matrix : Vec<Vec<T, N>, M> {
    // N: width, M:height
    // when using as a buffer for batches, vectors are actually *columns*
   public:
    Vec<T, N> &row(size_t m) { return this->at(m); }
    const Vec<T, N> &row(size_t m) const { return this->at(m); }

    Vec<T, M> get_column(size_t n) {
        Vec<T, M> res;
        for (unsigned m = 0; m < M; m++) { res[m] = row(m)[n]; }
        return res;
    }

    void set_column(size_t n, const Vec<T, M> &col) {
        for (unsigned m = 0; m < M; m++) { row(m)[n] = col[m]; }
    }

    std::array<T *, M> get_column_ptrs(size_t index) const {
        std::array<T *, M> ptrs;
        for (unsigned i = 0; i < M; i++) { ptrs[i] = &(row(i)[index]); }
        return ptrs;
    }
};
}  // namespace quicktex