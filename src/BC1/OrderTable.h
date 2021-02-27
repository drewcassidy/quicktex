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

#include <algorithm>
#include <array>
#include <cstdint>
#include <mutex>
#include <numeric>

#include "../Vector4.h"
#include "../util.h"
#include "tables.h"

namespace rgbcx {

template <size_t N> class OrderTable {
   public:
    using Hash = uint16_t;
    using FactorMatrix = std::array<float, 3>;

    class Histogram {
       public:
        Histogram() { _bins = {0}; }

        Histogram(std::array<uint8_t, 16> sels) {
            _bins = {0};
            for (unsigned i = 0; i < 16; i++) {
                assert(sels[i] < N);
                _bins[sels[i]]++;
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
            unsigned packed = 0;
            for (unsigned i = 0; i < (N - 1); i++) { packed |= (_bins[i] << (4 * i)); }

            assert(packed < TotalHashes);

            return packed;
        }

       private:
        std::array<uint8_t, N> _bins;
    };

    static inline constexpr size_t UniqueOrderings = (N == 4) ? NUM_UNIQUE_TOTAL_ORDERINGS4 : NUM_UNIQUE_TOTAL_ORDERINGS3;
    static inline constexpr size_t TotalHashes = (N == 4) ? 4096 : 256;

    static inline constexpr uint8_t GetUniqueOrdering(Hash hash, unsigned selector) {
        if constexpr (N == 4) { return g_unique_total_orders4[hash][selector]; }
        return g_unique_total_orders3[hash][selector];
    }

    static inline constexpr void GetUniqueOrderingSums(Hash hash, unsigned &f1, unsigned &f2, unsigned &f3) {
        f1 = GetUniqueOrdering(hash, 0);
        f2 = f1 + GetUniqueOrdering(hash, 1);
        f3 = f2 + GetUniqueOrdering(hash, 2);
    }

    OrderTable<N>() {
        static_assert(N == 4 || N == 3);

        const unsigned *weight_vals = (N == 4) ? g_weight_vals4 : g_weight_vals3;
        const float denominator = (N == 4) ? 3.0f : 2.0f;

        for (unsigned i = 0; i < UniqueOrderings; i++) {
            Histogram h;
            for (unsigned j = 0; j < N; j++) { h[j] = GetUniqueOrdering(i, j); }

            if (!h.Any16()) _hashes[h.GetPacked()] = (Hash)i;

            unsigned weight_accum = 0;
            for (unsigned sel = 0; sel < N; sel++) weight_accum += (weight_vals[sel] * h[sel]);

            // todo: use a Vector4 here instead for SIMD readiness
            float z00 = (float)((weight_accum >> 16) & 0xFF);
            float z10 = (float)((weight_accum >> 8) & 0xFF);
            float z11 = (float)(weight_accum & 0xFF);
            float z01 = z10;

            float det = z00 * z11 - z01 * z10;
            if (fabs(det) < 1e-8f) {
                _factors[i][0] = 0;
                _factors[i][1] = 0;
                _factors[i][2] = 0;
            } else {
                det = (denominator / 255.0f) / det;
                _factors[i][0] = z11 * det;
                _factors[i][1] = -z10 * det;
                _factors[i][2] = z00 * det;
            }
        }
    }

    Hash GetHash(Histogram &hist) const {
        for (unsigned i = 0; i < N; i++) {
            if (hist[i] == 16) return GetSingleColorHashes()[i];
        }

        return _hashes[hist.GetPacked()];
    }

    Vector4 GetFactors(Hash hash) { return Vector4(_factors[hash][0], _factors[hash][1], _factors[hash][1], _factors[hash][2]); }

    static inline constexpr std::array<Hash, N> GetSingleColorHashes() {
        if (N == 4) { return {15, 700, 753, 515}; }
        return {12, 15, 89};
    }

    static inline constexpr bool IsSingleColor(Hash hash) {
        auto hashes = GetSingleColorHashes();
        return (std::find(hashes.begin(), hashes.end(), hash) != hashes.end());
    }

   private:
    std::array<Hash, TotalHashes> _hashes;
    std::array<FactorMatrix, UniqueOrderings> _factors;
};

}  // namespace rgbcx