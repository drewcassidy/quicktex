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
#include <atomic>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <type_traits>

#include "../../Vector4.h"
#include "Histogram.h"

namespace quicktex::s3tc {
template <size_t N> class OrderTable {
   public:
    static constexpr unsigned HashCount = 1 << ((N - 1) * 4);     // 16**(N-1)
    static constexpr unsigned OrderCount = (N == 4) ? 969 : 153;  //(16+N-1)C(N-1)
#if RGBCX_USE_SMALLER_TABLES
    static constexpr unsigned BestOrderCount = 32;
#else
    static constexpr unsigned BestOrderCount = (N == 4) ? 128 : 32;
#endif

    using Hash = uint16_t;
    using OrderArray = std::array<Histogram<N>, OrderCount>;
    using BestOrderRow = std::array<Hash, BestOrderCount>;
    using BestOrderArray = std::array<BestOrderRow, OrderCount>;

    static std::atomic<bool> generated;

    static const OrderArray Orders;
    static const BestOrderArray BestOrders;
    static const std::array<Vector4, N> Weights;
    static const std::array<Hash, N> SingleColorHashes;

    static bool Generate() {
        static_assert(N == 4 || N == 3);

        table_mutex.lock();
        if (!generated) {
            hashes = new std::array<Hash, HashCount>();
            factors = new std::array<Vector4, OrderCount>();

            const float denominator = (N == 4) ? 3.0f : 2.0f;

            for (uint16_t i = 0; i < OrderCount; i++) {
                Histogram<N> h = Orders[i];
                if (!h.Any16()) hashes->at(h.GetPacked()) = i;

                Vector4 factor_matrix = 0;
                for (unsigned sel = 0; sel < N; sel++) factor_matrix += (Weights[sel] * h[sel]);

                float det = factor_matrix.Determinant2x2();
                if (fabs(det) < 1e-8f) {
                    factors->at(i) = Vector4(0);
                } else {
                    std::swap(factor_matrix[0], factor_matrix[3]);
                    factor_matrix *= Vector4(1, -1, -1, 1);
                    factor_matrix *= (denominator / 255.0f) / det;
                    factors->at(i) = factor_matrix;
                }
            }

            generated = true;
        }
        table_mutex.unlock();

        assert(generated);
        return true;
    }

    static Hash GetHash(Histogram<N> &hist) {
        for (unsigned i = 0; i < N; i++) {
            if (hist[i] == 16) return SingleColorHashes[i];
        }

        assert(generated);
        assert(hashes != nullptr);

        auto hash = hashes->at(hist.GetPacked());

        assert(hash < OrderCount);

        return hash;
    }

    static Vector4 GetFactors(Hash hash) {
        assert(generated.load());
        assert(factors != nullptr);

        return factors->at(hash);
    }

    static bool IsSingleColor(Hash hash) { return (std::find(SingleColorHashes.begin(), SingleColorHashes.end(), hash) != SingleColorHashes.end()); }

   private:
    static std::mutex table_mutex;
    static std::array<Hash, HashCount> *hashes;
    static std::array<Vector4, OrderCount> *factors;
};

template <> std::atomic<bool> OrderTable<3>::generated;
template <> std::atomic<bool> OrderTable<4>::generated;

template <> std::mutex OrderTable<3>::table_mutex;
template <> std::mutex OrderTable<4>::table_mutex;

template <> std::array<OrderTable<3>::Hash, OrderTable<3>::HashCount> *OrderTable<3>::hashes;
template <> std::array<OrderTable<4>::Hash, OrderTable<4>::HashCount> *OrderTable<4>::hashes;

template <> std::array<Vector4, OrderTable<3>::OrderCount> *OrderTable<3>::factors;
template <> std::array<Vector4, OrderTable<4>::OrderCount> *OrderTable<4>::factors;

template <> const std::array<Vector4, 3> OrderTable<3>::Weights;
template <> const std::array<Vector4, 4> OrderTable<4>::Weights;

template <> const std::array<uint16_t, 3> OrderTable<3>::SingleColorHashes;
template <> const std::array<uint16_t, 4> OrderTable<4>::SingleColorHashes;

template <> const OrderTable<3>::OrderArray OrderTable<3>::Orders;
template <> const OrderTable<4>::OrderArray OrderTable<4>::Orders;

template <> const OrderTable<3>::BestOrderArray OrderTable<3>::BestOrders;
template <> const OrderTable<4>::BestOrderArray OrderTable<4>::BestOrders;

extern template class OrderTable<3>;
extern template class OrderTable<4>;

}  // namespace quicktex::s3tc