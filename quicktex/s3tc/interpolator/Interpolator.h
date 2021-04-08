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
#include <array>    // for array
#include <cstdint>  // for uint8_t, uint16_t
#include <memory>   // for unique_ptr

#include "../../Color.h"  // for Color

namespace quicktex::s3tc {

class Interpolator {
   public:
    enum class Type { Ideal, IdealRound, Nvidia, AMD };

    static std::unique_ptr<Interpolator> MakeInterpolator(Type type = Type::Ideal);

    virtual ~Interpolator() noexcept = default;

    /**
     * Performs a 2/3 interpolation of a pair of 5-bit values to produce an 8-bit value
     * Output is approximately (2v0 + v1)/3, with v0 and v1 first extended to 8 bits.
     * @param v0 The first 5-bit value
     * @param v1 The second 5-bit value
     * @return The interpolated value
     */
    virtual uint8_t Interpolate5(uint8_t v0, uint8_t v1) const;

    /**
     * Performs a 2/3 interpolation of a pair of 5-bit values to produce an 8-bit value
     * Output is approximately (2v0 + v1)/3, with v0 and v1 first extended to 8 bits.
     * @param v0 The first 5-bit value
     * @param v1 The second 5-bit value
     * @return The interpolated value
     */
    virtual uint8_t Interpolate6(uint8_t v0, uint8_t v1) const;

    /**
     * Performs a 2/3 interpolation of a pair of 8-bit values to produce an 8-bit value
     * Output is approximately (2v0 + v1)/3.
     * Output is not guranteed to be accurate for the given interpolator if CanInterpolate8Bit() is false
     * @param v0 The first 8-bit value
     * @param v1 The second 8-bit value
     * @return The interpolated value
     */
    virtual uint8_t Interpolate8(uint8_t v0, uint8_t v1) const;

    /**
     * Performs a 1/2 interpolation of a pair of 5-bit values to produce an 8-bit value
     * Output is approximately (v0 + v1)/2, with v0 and v1 first extended to 8 bits.
     * @param v0 The first 5-bit value
     * @param v1 The second 5-bit value
     * @return The interpolated value
     */
    virtual uint8_t InterpolateHalf5(uint8_t v0, uint8_t v1) const;

    /**
     * Performs a 1/2 interpolation of a pair of 6-bit values to produce an 8-bit value
     * Output is approximately (v0 + v1)/2, with v0 and v1 first extended to 8 bits.
     * @param v0 The first 6-bit value
     * @param v1 The second 6-bit value
     * @return The interpolated value
     */
    virtual uint8_t InterpolateHalf6(uint8_t v0, uint8_t v1) const;

    /**
     * Performs a 1/2 interpolation of a pair of 8-bit values to produce an 8-bit value
     * Output is approximately (v0 + v1)/2.
     * Output is not guranteed to be accurate for the given interpolator if CanInterpolate8Bit() is false
     * @param v0 The first 8-bit value
     * @param v1 The second 8-bit value
     * @return The interpolated value
     */
    virtual uint8_t InterpolateHalf8(uint8_t v0, uint8_t v1) const;

    /**
     * Generates the 4 colors for a BC1 block from the given 5:6:5-packed colors
     * @param low first 5:6:5 color for the block
     * @param high second 5:6:5 color for the block
     * @param allow_3color if true, a different interpolation mode will be used if high >= low
     * @return an array of 4 Color values, with indices matching BC1 selectors
     */
    std::array<Color, 4> Interpolate565BC1(uint16_t low, uint16_t high, bool allow_3color = true) const;

    /**
     * Generates the 4 colors for a BC1 block from the given
     * @param low the first color for the block, as a seperated 5:6:5 Color object
     * @param high the second color for the block, as a seperated 5:6:5 Color object
     * @param use_3color if the 3-color interpolation mode should be used
     * @return an array of 4 Color values, with indices matching BC1 selectors
     */
    virtual std::array<Color, 4> InterpolateBC1(Color low, Color high, bool use_3color) const;

    /**
     * Gets the type of an interpolator
     * @return The interpolator type
     */
    virtual Type GetType() const noexcept { return Type::Ideal; }

    virtual bool CanInterpolate8Bit() const noexcept { return true; }

    /**
     * Checks if the interpolator uses an ideal algorithm
     * @return true if the interpolator is ideal, false otherwise.
     */
    virtual bool IsIdeal() const noexcept {
        auto type = GetType();
        return (type == Type::Ideal || type == Type::IdealRound);
    }

   private:
    Color InterpolateColor24(const Color &c0, const Color &c1) const {
        return Color(Interpolate8(c0.r, c1.r), Interpolate8(c0.g, c1.g), Interpolate8(c0.b, c1.b));
    }

    Color InterpolateHalfColor24(const Color &c0, const Color &c1) const {
        return Color(InterpolateHalf8(c0.r, c1.r), InterpolateHalf8(c0.g, c1.g), InterpolateHalf8(c0.b, c1.b));
    }
};

class InterpolatorRound final : public Interpolator {
   public:
    virtual uint8_t Interpolate5(uint8_t v0, uint8_t v1) const override;
    virtual uint8_t Interpolate6(uint8_t v0, uint8_t v1) const override;
    virtual uint8_t Interpolate8(uint8_t v0, uint8_t v1) const override;

    virtual Type GetType() const noexcept override { return Type::IdealRound; }
};

class InterpolatorNvidia final : public Interpolator {
   public:
    virtual uint8_t Interpolate5(uint8_t v0, uint8_t v1) const override;
    virtual uint8_t Interpolate6(uint8_t v0, uint8_t v1) const override;

    virtual uint8_t InterpolateHalf5(uint8_t v0, uint8_t v1) const override;
    virtual uint8_t InterpolateHalf6(uint8_t v0, uint8_t v1) const override;

    virtual std::array<Color, 4> InterpolateBC1(Color low, Color high, bool use_3color) const override;

    virtual Type GetType() const noexcept override { return Type::Nvidia; }
    virtual bool CanInterpolate8Bit() const noexcept override { return false; }

   private:
    Color InterpolateColor565(const Color &c0, const Color &c1) const {
        return Color(Interpolate5(c0.r, c1.r), Interpolate6(c0.g, c1.g), Interpolate5(c0.b, c1.b));
    }

    Color InterpolateHalfColor565(const Color &c0, const Color &c1) const {
        return Color(InterpolateHalf5(c0.r, c1.r), InterpolateHalf6(c0.g, c1.g), InterpolateHalf5(c0.b, c1.b));
    }
};

class InterpolatorAMD final : public Interpolator {
   public:
    virtual uint8_t Interpolate5(uint8_t v0, uint8_t v1) const override;
    virtual uint8_t Interpolate6(uint8_t v0, uint8_t v1) const override;
    virtual uint8_t Interpolate8(uint8_t v0, uint8_t v1) const override;

    virtual uint8_t InterpolateHalf5(uint8_t v0, uint8_t v1) const override;
    virtual uint8_t InterpolateHalf6(uint8_t v0, uint8_t v1) const override;
    virtual uint8_t InterpolateHalf8(uint8_t v0, uint8_t v1) const override;

    virtual Type GetType() const noexcept override { return Type::AMD; }
};
}  // namespace quicktex::s3tc