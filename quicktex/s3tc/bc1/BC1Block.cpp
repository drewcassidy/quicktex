/*  Quicktex Texture Compression Library
    Copyright (C) 2021-2024 Andrew Cassidy <drewcassidy@me.com>
    Partially derived from rgbcx.h written by Richard Geldreich <richgel99@gmail.com>
    and licenced under the public domain

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
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
