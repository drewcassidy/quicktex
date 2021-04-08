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
#include <initializer_list>
#include <mutex>
#include <numeric>

#include "../../Vector4.h"
#include "../../util.h"

namespace quicktex::s3tc  {
template <size_t N> class Histogram {
   public:
    using Hash = uint16_t;

    Histogram() { _bins.fill(0); }

    Histogram(std::array<uint8_t, 16> sels) {
        _bins.fill(0);
        for (unsigned i = 0; i < 16; i++) {
            assert(sels[i] < N);
            _bins[sels[i]]++;
        }
    }

    Histogram(std::initializer_list<uint8_t> init) {
        assert(init.size() <= N);
        _bins.fill(0);
        auto item = init.begin();
        for (unsigned i = 0; i < init.size(); i++) {
            _bins[i] = *item;
            item++;
        }
    }

    uint8_t operator[](size_t index) const {
        assert(index < N);
        return _bins[index];
    }
    uint8_t &operator[](size_t index) {
        assert(index < N);
        return _bins[index];
    }

    bool Any16() {
        return std::any_of(_bins.begin(), _bins.end(), [](int i) { return i == 16; });
    }

    unsigned GetPacked() const {
        Hash packed = 0;

        for (unsigned i = 0; i < (N-1); i++) {
            assert(_bins[i] <= (1U << 4) - 1U);
            packed |= static_cast<uint16_t>(_bins[i]) << (i * 4U);
        }

        return packed;
    }

   private:
    std::array<uint8_t, N> _bins;
};
}  // namespace quicktex::s3tc