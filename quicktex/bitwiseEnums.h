/*  Quicktex Texture Compression Library
    Copyright (C) 2021-2024 Andrew Cassidy <drewcassidy@me.com>
    Partially derived from rgbcx.h written by Richard Geldreich 2020 <richgel99@gmail.com>
    and licenced under the public domain

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