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

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma once

#include <array>
#include <tuple>
#include <xsimd/xsimd.hpp>

#include "util/ranges.h"

namespace quicktex {

namespace detail {

template <typename T>
concept simdable = subscriptable_range<T> && std::contiguous_iterator<decltype(std::declval<T>().begin())> &&
                   std::is_arithmetic_v<range_value_t<T>>;
template <typename T, bool serial = false> struct chunker_impl {};

template <typename T, bool serial>
    requires simdable<T> && (!serial)
struct chunker_impl<T, serial> {
    // range with contiguous, SIMDable data

    static constexpr size_t steps = 2;
    using chunk_types = std::tuple<xsimd::batch<range_value_t<T>>, range_value_t<T>>;

    template <size_t step> using chunk_type = std::tuple_element_t<step, chunk_types>;
    static constexpr std::array<size_t, 2> chunk_sizes = {chunk_type<0>::size, 1};

    template <size_t step> static constexpr size_t chunk_count(const T& r) {
        if constexpr (step == 0) {
            return std::size(r) / chunk_sizes[0];
        } else {
            return std::size(r) % chunk_sizes[0];
        }
    }

    template <size_t step> static constexpr auto get_chunk(const T& r, size_t i) {
        assert(i < chunk_count<step>(r));
        if constexpr (step == 0) {
            return xsimd::load_unaligned(&r[chunk_sizes[0] * i]);
        } else {
            return r[chunk_sizes[0] * chunk_count<0>(r) + i];
        }
    }

    template <size_t step>
    static constexpr void set_chunk(T& r, size_t i, const std::tuple_element_t<step, chunk_types>& c) {
        assert(i < chunk_count<step>(r));
        if constexpr (step == 0) {
            xsimd::store_unaligned(&r[chunk_sizes[0] * i], c);
        } else {
            r[chunk_sizes[0] * chunk_count<0>(r) + i] = c;
        }
    }
};

template <typename T, bool serial>
    requires subscriptable_range<T> && (!simdable<T> || serial)
struct chunker_impl<T, serial> {
    // range with data that cant be SIMDed
    static constexpr size_t steps = 1;
    template <size_t step> using chunk_type = range_value_t<T>;
    static constexpr std::array<size_t, 1> chunk_sizes = {1};

    template <size_t step> static constexpr size_t chunk_count(const T& r) { return r.size(); }
    template <size_t step> static constexpr auto get_chunk(const T& r, size_t i) { return r[i]; }
    template <size_t step> static constexpr void set_chunk(T& r, size_t i, const chunk_type<0>& c) { r[i] = c; }
};

template <typename T, bool serial>
    requires(!sized_range<T>)
struct chunker_impl<T, serial> {
    static constexpr size_t steps = 1;
    using chunk_types = std::tuple<T>;
    template <size_t step> using chunk_type = T;

    static constexpr std::array<size_t, 1> chunk_sizes = {1};

    template <size_t step> static constexpr size_t chunk_count(const T& r) { return 1; }
    template <size_t step> static constexpr auto get_chunk(const T& r, size_t i) { return r; }
    template <size_t step> static constexpr void set_chunk(T& r, size_t i, const T& c) { r = c; }
};

template <typename T, bool serial = false, size_t step = 0>
using chunk_type = typename chunker_impl<T, serial>::template chunk_type<step>;

template <typename T, bool serial, typename Op, std::size_t step, typename... Args>
static constexpr bool callable_step() {
    return std::is_invocable_r_v<typename chunker_impl<T, serial>::template chunk_type<step>, Op,
                                 typename chunker_impl<Args, serial>::template chunk_type<step>...>;
}

template <typename T, bool serial, typename Op, typename... Args, std::size_t... steps>
static constexpr bool callable_steps(std::index_sequence<steps...>) {
    return (callable_step<T, serial, Op, steps, Args...>() && ...);
}

template <typename T, bool serial, typename Op, typename... Args> static constexpr bool callable() {
    //    if constexpr (!(std::same_as<T, Args> && ...)) return false;
    //    return callable_steps<T, serial, Op>(std::make_index_sequence<chunker_impl<T, serial>::steps>());
    return callable_steps<T, serial, Op, Args...>(std::make_index_sequence<1>());
}

template <typename T, bool serial, size_t step, typename... Args>
    requires((std::is_scalar_v<Args> || std::same_as<T, Args>) && ...)
inline void do_map_step(auto f, T& result, const Args&... args) {
    using impl = chunker_impl<T, serial>;
    using chunk_type = typename impl::template chunk_type<step>;
    size_t chunk_count = impl::template chunk_count<step>(result);

    for (unsigned i = 0; i < chunk_count; i++) {
        chunk_type out_chunk = f(impl::template get_chunk<step>(args, i)...);
        impl::template set_chunk<step>(result, i, out_chunk);
    }
}

template <typename T, bool serial, typename Op, std::size_t... steps, typename... Args>
    requires((std::is_scalar_v<Args> || std::same_as<T, Args>) && ...)
inline void do_map_steps(Op f, T& result, std::index_sequence<steps...>, const Args&... args) {
    //    static_assert(callable<T, serial, Op, Args...>());

    (do_map_step<T, serial, steps>(f, result, args...), ...);
}

template <typename T, bool serial, typename Op, typename... Args>
    requires((std::is_scalar_v<Args> || std::same_as<T, Args>) && ...)
inline void do_map_all(Op f, T& result, const Args&... args) {
    constexpr bool must_serialize = serial || !callable<T, false, Op, Args...>();
    do_map_steps<T, must_serialize>(f, result, std::make_index_sequence<chunker_impl<T, serial>::steps>(), args...);
}
}  // namespace detail

template <typename R, typename T, bool serial = false, typename Op, typename... Args>
    requires sized_range<T> && (sized_range<Args> && ...)
inline R map_to(Op f, const T& in, const Args&... args) {
    // the input and result types are not the same, so attempting chunking is unsafe
    R result{};
    for (unsigned i = 0; i < in.size(); i++) { result[i] = f(in[i], args[i]...); }
    return result;
}

template <typename T, bool serial = false, typename Op, typename... Args>
    requires sized_range<T>
inline auto map(Op f, const T& in, const Args&... args) {
    //    assert(((in.size() == args.size())) && ...);

    if constexpr (((std::is_scalar_v<Args> || std::same_as<T, Args>)&&...) &&
                  (detail::callable<T, true, Op, T, Args...>())) {
        // the input and result types are all the same type and size, so we can attempt chunking
        T result{};
        detail::do_map_all<T, serial>(f, result, in, args...);
        return result;
    } else {
        using result_type = std::invoke_result_t<Op, typename detail::chunk_type<T, true>, range_value_t<Args>...>;
        return map_to<std::array<result_type, std::tuple_size_v<T>>, T, serial>(f, in, args...);
    }
}

}  // namespace quicktex
#pragma clang diagnostic pop