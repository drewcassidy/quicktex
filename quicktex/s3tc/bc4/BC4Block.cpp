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

#include "BC4Block.h"

#include <algorithm>
#include <stdexcept>

#include "../../util.h"

namespace quicktex::s3tc {

BC4Block::SelectorArray BC4Block::GetSelectors() const {
    auto packed = Pack<uint8_t, uint64_t, 8, SelectorSize>(_selectors);
    auto rows = Unpack<uint64_t, uint16_t, SelectorBits * Width, Height>(packed);
    return MapArray(rows, Unpack<uint16_t, uint8_t, SelectorBits, Width>);
}

void BC4Block::SetSelectors(const BC4Block::SelectorArray& unpacked) {
    for (unsigned y = 0; y < (unsigned)Height; y++) {
        if (std::any_of(unpacked[y].begin(), unpacked[y].end(), [](uint8_t i) { return i > SelectorMax; }))
            throw std::invalid_argument("Selector value out of bounds.");
    }
    auto rows = MapArray(unpacked, Pack<uint8_t, uint16_t, SelectorBits, Width>);
    auto packed = Pack<uint16_t, uint64_t, SelectorBits * Width, Height>(rows);
    _selectors = Unpack<uint64_t, uint8_t, 8, SelectorSize>(packed);
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

bool BC4Block::operator==(const BC4Block& Rhs) const { return alpha0 == Rhs.alpha0 && alpha1 == Rhs.alpha1 && _selectors == Rhs._selectors; }
bool BC4Block::operator!=(const BC4Block& Rhs) const { return !(Rhs == *this); }
}  // namespace quicktex::s3tc
