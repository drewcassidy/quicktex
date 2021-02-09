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
#include <cassert>
#include <cstdint>
#include <memory>

#include "util.h"

#ifdef NDEBUG  // asserts disabled
static constexpr bool ndebug = true;
#else  // asserts enabled
static constexpr bool ndebug = false;
#endif

namespace rgbcx {

template <size_t size, int op(int)> static constexpr std::array<uint8_t, size> ExpandArray() {
    std::array<uint8_t, size> res;
    for (int i = 0; i < size; i++) { res[i] = op(i); }
    return res;
}

class Interpolator {
   public:
    struct MatchEntry {
        uint8_t high;
        uint8_t low;
        uint8_t error;
    };

    Interpolator();
    virtual ~Interpolator() noexcept = default;

    virtual int Interpolate5(int v0, int v1) = 0;
    virtual int Interpolate6(int v0, int v1) = 0;
    virtual int InterpolateHalf5(int v0, int v1) = 0;
    virtual int InterpolateHalf6(int v0, int v1) = 0;

    constexpr MatchEntry GetMatch5(int i) noexcept(ndebug) {
        assert(i < match_count);
        return (*_single_match5)[i];
    }
    constexpr MatchEntry GetMatch6(int i) noexcept(ndebug) {
        assert(i < match_count);
        return (*_single_match6)[i];
    }
    constexpr MatchEntry GetMatchHalf5(int i) noexcept(ndebug) {
        assert(i < match_count);
        return (*_single_match5_half)[i];
    }
    constexpr MatchEntry GetMatchHalf6(int i) noexcept(ndebug) {
        assert(i < match_count);
        return (*_single_match6_half)[i];
    }

   private:
    constexpr static inline size_t size5 = 32;
    constexpr static inline size_t size6 = 64;
    constexpr static inline size_t match_count = 256;

    constexpr static auto Expand5 = ExpandArray<size5, scale5To8>();
    constexpr static auto Expand6 = ExpandArray<size6, scale6To8>();

    // match tables used for single-color blocks
    using MatchList = std::array<MatchEntry, match_count>;
    using MatchListPtr = std::shared_ptr<MatchList>;

    const MatchListPtr _single_match5 = {std::make_shared<MatchList>()};
    const MatchListPtr _single_match6 = {std::make_shared<MatchList>()};
    const MatchListPtr _single_match5_half = {std::make_shared<MatchList>()};
    const MatchListPtr _single_match6_half = {std::make_shared<MatchList>()};

    virtual constexpr bool isIdeal() noexcept { return false; }
    virtual constexpr bool useExpandedInMatch() noexcept { return true; }

    void PrepSingleColorTables(const MatchListPtr &matchTable, const MatchListPtr &matchTableHalf, int len);

    int PrepSingleColorTableEntry(const MatchListPtr &matchTable, int v, int i, int low, int high, int low_e, int high_e, int lowest_error, bool half,
                                  bool ideal);
};

class InterpolatorIdeal : public Interpolator {
   public:
    virtual int Interpolate5(int v0, int v1) const;
    virtual int Interpolate6(int v0, int v1) const;
    virtual int InterpolateHalf5(int v0, int v1) const;
    virtual int InterpolateHalf6(int v0, int v1) const;

   private:
    int Interpolate5or6(int v0, int v1) const;
    int InterpolateHalf5or6(int v0, int v1) const;
    virtual constexpr bool isIdeal() noexcept override { return true; }
};

class InterpolatorIdealRound : public InterpolatorIdeal {
   public:
    virtual int Interpolate5(int v0, int v1) const override;
    virtual int Interpolate6(int v0, int v1) const override;

   private:
    int Interpolate5or6Round(int v0, int v1) const;
};

class InterpolatorNvidia : public Interpolator {
   public:
    virtual int Interpolate5(int v0, int v1) const;
    virtual int Interpolate6(int v0, int v1) const;
    virtual int InterpolateHalf5(int v0, int v1) const;
    virtual int InterpolateHalf6(int v0, int v1) const;

   private:
    virtual constexpr bool useExpandedInMatch() noexcept override { return false; }
};

class InterpolatorAMD : public Interpolator {
   public:
    virtual int Interpolate5(int v0, int v1) const;
    virtual int Interpolate6(int v0, int v1) const;
    virtual int InterpolateHalf5(int v0, int v1) const;
    virtual int InterpolateHalf6(int v0, int v1) const;

   private:
    int Interpolate5or6(int v0, int v1) const;
    int InterpolateHalf5or6(int v0, int v1) const;
};
}  // namespace rgbcx