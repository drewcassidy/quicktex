/*  Quicktex Texture Compression Library
    Copyright (C) 2021 Andrew Cassidy <drewcassidy@me.com>
    Partially derived from rgbcx.h written by Richard Geldreich 2020 <richgel99@gmail.com>
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

#include <type_traits>

// Thanks dkavolis
template <typename E, typename = std::enable_if_t<std::is_enum_v<E>>> constexpr inline auto operator~(E a) noexcept -> E {
    using Base = std::underlying_type_t<E>;
    return static_cast<E>(~static_cast<Base>(a));
}

template <typename E, typename = std::enable_if_t<std::is_enum_v<E>>> constexpr inline auto operator|(E a, E b) noexcept -> E {
    using Base = std::underlying_type_t<E>;
    return static_cast<E>(static_cast<Base>(a) | static_cast<Base>(b));
}

template <typename E, typename = std::enable_if_t<std::is_enum_v<E>>> constexpr inline auto operator&(E a, E b) noexcept -> E {
    using Base = std::underlying_type_t<E>;
    return static_cast<E>(static_cast<Base>(a) & static_cast<Base>(b));
}

template <typename E, typename = std::enable_if_t<std::is_enum_v<E>>> constexpr inline auto operator^(E a, E b) noexcept -> E {
    using Base = std::underlying_type_t<E>;
    return static_cast<E>(static_cast<Base>(a) ^ static_cast<Base>(b));
}

template <typename E, typename = std::enable_if_t<std::is_enum_v<E>>> constexpr inline auto operator|=(E& a, E b) noexcept -> E& {
    a = a | b;
    return a;
}

template <typename E, typename = std::enable_if_t<std::is_enum_v<E>>> constexpr inline auto operator&=(E& a, E b) noexcept -> E& {
    a = a & b;
    return a;
}

template <typename E, typename = std::enable_if_t<std::is_enum_v<E>>> constexpr inline auto operator^=(E& a, E b) noexcept -> E& {
    a = a ^ b;
    return a;
}