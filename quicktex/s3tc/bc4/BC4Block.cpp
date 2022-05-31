/*  Quicktex Texture Compression Library
    Copyright (C) 2021-2022 Andrew Cassidy <drewcassidy@me.com>
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

#include "BC4Block.h"

#include <algorithm>
#include <stdexcept>

#include "../../util.h"

namespace quicktex::s3tc {

BC4Block::SelectorArray BC4Block::GetSelectors() const {
    auto packed = pack<uint64_t>(_selectors, 8);
    auto rows = unpack<uint16_t, Height>(packed, SelectorBits * Width);
    return MapArray(rows, [](auto row) { return unpack<uint8_t, Width>(row, SelectorBits); });
}

void BC4Block::SetSelectors(const BC4Block::SelectorArray& unpacked) {
    for (unsigned y = 0; y < (unsigned)Height; y++) {
        if (std::any_of(unpacked[y].begin(), unpacked[y].end(), [](uint8_t i) { return i > SelectorMax; }))
            throw std::invalid_argument("Selector value out of bounds.");
    }
    auto rows = MapArray(unpacked, [](auto r) { return pack<uint16_t>(r, SelectorBits); });
    auto packed = pack<uint64_t>(rows, SelectorBits * Width);
    _selectors = unpack<uint8_t, SelectorSize>(packed, 8);
}

std::array<uint8_t, 8> BC4Block::GetValues6() const {
    return {alpha0,
            alpha1,
            static_cast<uint8_t>((alpha0 * 4 + alpha1) / 5),
            static_cast<uint8_t>((alpha0 * 3 + alpha1 * 2) / 5),
            static_cast<uint8_t>((alpha0 * 2 + alpha1 * 3) / 5),
            static_cast<uint8_t>((alpha0 + alpha1 * 4) / 5),
            0,
            0xFF};
}

std::array<uint8_t, 8> BC4Block::GetValues8() const {
    return {alpha0,
            alpha1,
            static_cast<uint8_t>((alpha0 * 6 + alpha1) / 7),
            static_cast<uint8_t>((alpha0 * 5 + alpha1 * 2) / 7),
            static_cast<uint8_t>((alpha0 * 4 + alpha1 * 3) / 7),
            static_cast<uint8_t>((alpha0 * 3 + alpha1 * 4) / 7),
            static_cast<uint8_t>((alpha0 * 2 + alpha1 * 5) / 7),
            static_cast<uint8_t>((alpha0 + alpha1 * 6) / 7)};
}

bool BC4Block::operator==(const BC4Block& Rhs) const {
    return alpha0 == Rhs.alpha0 && alpha1 == Rhs.alpha1 && _selectors == Rhs._selectors;
}
bool BC4Block::operator!=(const BC4Block& Rhs) const { return !(Rhs == *this); }
}  // namespace quicktex::s3tc
