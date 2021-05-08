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

#include "BC1Block.h"

#include <stdexcept>
#include <algorithm>

#include "../../util.h"

namespace quicktex::s3tc {
uint16_t BC1Block::GetColor0Raw() const { return Pack<uint8_t, uint16_t, 8, EndpointSize>(_color0); }
uint16_t BC1Block::GetColor1Raw() const { return Pack<uint8_t, uint16_t, 8, EndpointSize>(_color1); }

void BC1Block::SetColor0Raw(uint16_t c) { _color0 = Unpack<uint16_t, uint8_t, 8, EndpointSize>(c); }
void BC1Block::SetColor1Raw(uint16_t c) { _color1 = Unpack<uint16_t, uint8_t, 8, EndpointSize>(c); }

BC1Block::SelectorArray BC1Block::GetSelectors() const { return MapArray(_selectors, Unpack<uint8_t, uint8_t, SelectorBits, Width>); }

void BC1Block::SetSelectors(const BC1Block::SelectorArray& unpacked) {
    for (unsigned y = 0; y < (unsigned)Height; y++) {
        if (std::any_of(unpacked[y].begin(), unpacked[y].end(), [](uint8_t i) { return i > SelectorMax; }))
            throw std::invalid_argument("Selector value out of bounds.");
    }
    _selectors = MapArray(unpacked, Pack<uint8_t, uint8_t, SelectorBits, Width>);
}

bool BC1Block::operator==(const BC1Block& Rhs) const { return _color0 == Rhs._color0 && _color1 == Rhs._color1 && _selectors == Rhs._selectors; }
bool BC1Block::operator!=(const BC1Block& Rhs) const { return !(Rhs == *this); }

}  // namespace quicktex::s3tc
