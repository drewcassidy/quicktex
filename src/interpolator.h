/*  Python-rgbcx Texture Compression Library
    Copyright (C) 2021 Andrew Cassidy <drewcassidy@me.com>
    Partially derived from rgbcx.h written by Richard Geldreich <richgel99@gmail.com>
    and licenced under the public domain

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

namespace rgbcx {

class Interpolator {
   public:
    virtual int Interpolate5(int v0, int v1) = 0;
    virtual int Interpolate6(int v0, int v1) = 0;
    virtual int InterpolateHalf5(int v0, int v1) = 0;
    virtual int InterpolateHalf6(int v0, int v1) = 0;
    virtual ~Interpolator() noexcept = default;
};

class InterpolatorIdeal : public Interpolator {
   public:
    virtual int Interpolate5(int v0, int v1);
    virtual int Interpolate6(int v0, int v1);
    virtual int InterpolateHalf5(int v0, int v1);
    virtual int InterpolateHalf6(int v0, int v1);

   private:
    int Interpolate5or6(int v0, int v1);
    int InterpolateHalf5or6(int v0, int v1);
};

class InterpolatorIdealRound : public InterpolatorIdeal {
   public:
    virtual int Interpolate5(int v0, int v1);
    virtual int Interpolate6(int v0, int v1);

   private:
    int Interpolate5or6Round(int v0, int v1);
};

class InterpolatorNvidia : public Interpolator {
   public:
    virtual int Interpolate5(int v0, int v1);
    virtual int Interpolate6(int v0, int v1);
    virtual int InterpolateHalf5(int v0, int v1);
    virtual int InterpolateHalf6(int v0, int v1);
};

class InterpolatorAMD : public Interpolator {
   public:
    virtual int Interpolate5(int v0, int v1);
    virtual int Interpolate6(int v0, int v1);
    virtual int InterpolateHalf5(int v0, int v1);
    virtual int InterpolateHalf6(int v0, int v1);

   private:
    int Interpolate5or6(int v0, int v1);
    int InterpolateHalf5or6(int v0, int v1);
};
}  // namespace rgbcx