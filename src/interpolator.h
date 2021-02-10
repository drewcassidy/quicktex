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

#include "color.h"
#include "ndebug.h"
#include "util.h"

namespace rgbcx {

template <size_t Size, int Op(int)> static constexpr std::array<uint8_t, Size> ExpandArray() {
    std::array<uint8_t, Size> res;
    for (int i = 0; i < Size; i++) { res[i] = Op(i); }
    return res;
}

class Interpolator {
   public:
    //    struct MatchEntry {
    //        uint8_t high;
    //        uint8_t low;
    //        uint8_t error;
    //    };

    virtual ~Interpolator() noexcept = default;

    /**
     * Performs a 2/3 interpolation of a pair of 5-bit values to produce an 8-bit value
     * Output is approximately (2v0 + v1)/3, with v0 and v1 first extended to 8 bits.
     * @param v0 The first 5-bit value
     * @param v1 The second 5-bit value
     * @return The interpolated value
     */
    virtual int Interpolate5(int v0, int v1) const;

    /**
     * Performs a 2/3 interpolation of a pair of 5-bit values to produce an 8-bit value
     * Output is approximately (2v0 + v1)/3, with v0 and v1 first extended to 8 bits.
     * @param v0 The first 5-bit value
     * @param v1 The second 5-bit value
     * @return The interpolated value
     */
    virtual int Interpolate6(int v0, int v1) const;

    /**
     * Performs a 1/2 interpolation of a pair of 5-bit values to produce an 8-bit value
     * Output is approximately (v0 + v1)/2, with v0 and v1 first extended to 8 bits.
     * @param v0 The first 5-bit value
     * @param v1 The second 5-bit value
     * @return The interpolated value
     */
    virtual int InterpolateHalf5(int v0, int v1) const;

    /**
     * Performs a 1/2 interpolation of a pair of 6-bit values to produce an 8-bit value
     * Output is approximately (v0 + v1)/2, with v0 and v1 first extended to 8 bits.
     * @param v0 The first 6-bit value
     * @param v1 The second 6-bit value
     * @return The interpolated value
     */
    virtual int InterpolateHalf6(int v0, int v1) const;

    /**
     * Generates the 4 colors for a BC1 block from the given 5:6:5-packed colors
     * @param low first 5:6:5 color for the block
     * @param high second 5:6:5 color for the block
     * @return and array of 4 Color32 values, with indices matching BC1 selectors
     */
    virtual std::array<Color32, 4> InterpolateBC1(uint16_t low, uint16_t high) const;

   private:
    virtual int Interpolate8(int v0, int v1) const;
    virtual int InterpolateHalf8(int v0, int v1) const;

    //    constexpr static auto Expand5 = ExpandArray<Size5, scale5To8>();
    //    constexpr static auto Expand6 = ExpandArray<size6, scale6To8>();
    //
    //    // match tables used for single-color blocks
    //    using MatchList = std::array<MatchEntry, match_count>;
    //    using MatchListPtr = std::shared_ptr<MatchList>;
    //
    //    const MatchListPtr _single_match5 = {std::make_shared<MatchList>()};
    //    const MatchListPtr _single_match6 = {std::make_shared<MatchList>()};
    //    const MatchListPtr _single_match5_half = {std::make_shared<MatchList>()};
    //    const MatchListPtr _single_match6_half = {std::make_shared<MatchList>()};

    Color32 InterpolateColor24(const Color32 &c0, const Color32 &c1) const {
        return Color32(Interpolate8(c0.r, c1.r), Interpolate8(c0.g, c1.g), Interpolate8(c0.b, c1.b));
    }

    Color32 InterpolateHalfColor24(const Color32 &c0, const Color32 &c1) const {
        return Color32(InterpolateHalf8(c0.r, c1.r), InterpolateHalf8(c0.g, c1.g), InterpolateHalf8(c0.b, c1.b));
    }

    virtual constexpr bool isIdeal() noexcept { return true; }
    //    virtual constexpr bool useExpandedInMatch() noexcept { return true; }
    //
    //    void PrepSingleColorTables(const MatchListPtr &matchTable, const MatchListPtr &matchTableHalf, int len);
    //
    //    int PrepSingleColorTableEntry(const MatchListPtr &matchTable, int v, int i, int low, int high, int low_e, int high_e, int lowest_error, bool half,
    //                                  bool ideal);
};

class InterpolatorRound : public Interpolator {
   public:
    int Interpolate5(int v0, int v1) const override;
    int Interpolate6(int v0, int v1) const override;

   private:
    int Interpolate8(int v0, int v1) const override;
};

class InterpolatorNvidia : public Interpolator {
   public:
    int Interpolate5(int v0, int v1) const override;
    int Interpolate6(int v0, int v1) const override;
    int InterpolateHalf5(int v0, int v1) const override;
    int InterpolateHalf6(int v0, int v1) const override;
    std::array<Color32, 4> InterpolateBC1(uint16_t low, uint16_t high) const override;
    constexpr bool isIdeal() noexcept override { return false; }

   private:
    Color32 InterpolateColor565(const Color32 &c0, const Color32 &c1) const {
        return Color32(Interpolate5(c0.r, c1.r), Interpolate6(c0.g, c1.g), Interpolate5(c0.b, c1.b));
    }

    Color32 InterpolateHalfColor565(const Color32 &c0, const Color32 &c1) const {
        return Color32(InterpolateHalf5(c0.r, c1.r), InterpolateHalf6(c0.g, c1.g), InterpolateHalf5(c0.b, c1.b));
    }
};

class InterpolatorAMD : public Interpolator {
   public:
    int Interpolate5(int v0, int v1) const override;
    int Interpolate6(int v0, int v1) const override;
    int InterpolateHalf5(int v0, int v1) const override;
    int InterpolateHalf6(int v0, int v1) const override;
    constexpr bool isIdeal() noexcept override { return false; }

   private:
    int Interpolate8(int v0, int v1) const override;
    int InterpolateHalf8(int v0, int v1) const override;
};
}  // namespace rgbcx