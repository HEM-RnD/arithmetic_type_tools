/**@file
 * @brief C++ tools for arithmetic type fitting, selection and size-mapping in the form of a header-only library
 * @author Stefan Hamminga <s@stefanhamminga.com>
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>
#include <algorithm>

namespace arithmetic_type_tools {
    /**@{ Select an integral or floating point type by size */
    template <size_t S> struct signed_by_size { using type = void; };
    template <> struct signed_by_size<1> { using type = int8_t; };
    template <> struct signed_by_size<2> { using type = int16_t; };
    template <> struct signed_by_size<4> { using type = int32_t; };
    template <> struct signed_by_size<8> { using type = int64_t; };
    #ifdef __SIZEOF_INT128__
        template <> struct signed_by_size<16> { using type = __int128_t; };
    #endif
    #ifdef __SIZEOF_INT256__
        template <> struct signed_by_size<32> { using type = __int256_t; };
    #endif

    template <size_t S>
    using signed_by_size_t = typename signed_by_size<S>::type;

    template <size_t S> struct unsigned_by_size { using type = void; };
    template <> struct unsigned_by_size<1> { using type = uint8_t; };
    template <> struct unsigned_by_size<2> { using type = uint16_t; };
    template <> struct unsigned_by_size<4> { using type = uint32_t; };
    template <> struct unsigned_by_size<8> { using type = uint64_t; };
    #ifdef __SIZEOF_INT128__
        template <> struct unsigned_by_size<16> { using type = __uint128_t; };
    #endif
    #ifdef __SIZEOF_INT256__
        template <> struct unsigned_by_size<32> { using type = __uint256_t; };
    #endif

    template <size_t S>
    using unsigned_by_size_t = typename unsigned_by_size<S>::type;

    // The explicit size checks prevent ambiguity in (long) double sizes.
    template <size_t S> struct float_by_size { using type = void; };
    #if __SIZEOF_FLOAT__ == 4
        template <> struct float_by_size<4> { using type = float; };
    #endif
    #if __SIZEOF_DOUBLE__ == 8
        template <> struct float_by_size<8> { using type = double; };
    #endif
    #if __SIZEOF_LONG_DOUBLE__ == 16
        template <> struct float_by_size<16> { using type = long double; };
    #endif

    template <size_t S>
    using float_by_size_t = typename float_by_size<S>::type;
    /** @} */

    // Yes, it's not pretty, but it solves the chicken-egg issue caused by the broken `std::common_type`
    /**
     * Variadic `min` implementation for safely mixing unsigned, signed and floating point arguments
     * @returns Lowest value of type `fit_all_t<T...>`
     */
    template <typename... Ts,
              typename T = std::conditional_t<(std::is_signed_v<Ts> || ...) && (std::is_unsigned_v<Ts> || ...),
                                              signed_by_size_t<sizeof(std::common_type_t<Ts...>) *
                                                               ((sizeof(std::common_type_t<Ts...>) > std::max(
                                                                    { (std::is_unsigned_v<Ts> ? sizeof(Ts) : 0)... }
                                                                    )) ? 1 : 2)>,
                                              std::common_type_t<Ts...>>>
    constexpr T min(const Ts&... vals) {
        T temp = std::numeric_limits<T>::max();
        ((temp = static_cast<T>(temp) < static_cast<T>(vals) ? static_cast<T>(temp) : static_cast<T>(vals)), ...);
        return temp;
    }

    /**
     * Variadic `max` implementation for safely mixing unsigned, signed and floating point arguments
     * @returns Highest value of type `fit_all_t<T...>`
     */
    template <typename... Ts,
              typename T = std::conditional_t<(std::is_signed_v<Ts> || ...) && (std::is_unsigned_v<Ts> || ...),
                                              signed_by_size_t<sizeof(std::common_type_t<Ts...>) *
                                                               ((sizeof(std::common_type_t<Ts...>) > std::max(
                                                                    { (std::is_unsigned_v<Ts> ? sizeof(Ts) : 0)... }
                                                                    )) ? 1 : 2)>,
                                              std::common_type_t<Ts...>>>
    constexpr T max(const Ts... vals) {
        T temp = std::numeric_limits<T>::lowest();
        ((temp = static_cast<T>(vals) > static_cast<T>(temp) ? static_cast<T>(vals) : static_cast<T>(temp)), ...);
        return temp;
    }

    /**
     * `clamp` implementation for safely mixing unsigned, signed and floating point arguments
     * @returns Value of type `fit_all_t<T...>` clamped to `low...high`
     */
    template <typename T, typename U, typename V,
              typename C = std::conditional_t<(  std::is_signed_v<T> ||   std::is_signed_v<U> ||   std::is_signed_v<V>) &&
                                              (std::is_unsigned_v<T> || std::is_unsigned_v<U> || std::is_unsigned_v<V>),
                                              signed_by_size_t<sizeof(std::common_type_t<T, U, V>) *
                                                               ((sizeof(std::common_type_t<T, U, V>) > std::max(
                                                                    {(std::is_unsigned_v<T> ? sizeof(T) : 0),
                                                                     (std::is_unsigned_v<U> ? sizeof(U) : 0),
                                                                     (std::is_unsigned_v<V> ? sizeof(V) : 0)}
                                                                    )
                                                                ) ? 1 : 2)>,
                                              std::common_type_t<T, U, V>>>
    constexpr auto clamp(const T& low, const U& val, const V& high) {
        return static_cast<C>(low) < static_cast<C>(val)
                    ? (static_cast<C>(val) < static_cast<C>(high)
                            ? static_cast<C>(val)
                            : static_cast<C>(high))
                    : static_cast<C>(low);
    }

    /** Store the size of the largest signed type in a range of types in `value`. */
    template <typename... T>
    struct largest_signed {
        static constexpr size_t value = max((std::is_signed_v<T> ? sizeof(T) : 0)...);
    };

    /** Return the size of the largest signed type in a range of types. */
    template <typename... T>
    constexpr decltype(auto) largest_signed_v = largest_signed<T...>::value;

    /** Store the size of the largest unsigned type in a range of types in `value`. */
    template <typename... T>
    struct largest_unsigned {
        static constexpr size_t value = max((std::is_unsigned_v<T> ? sizeof(T) : 0)...);
    };

    /** Return the size of the largest signed type in a range of types. */
    template <typename... T>
    constexpr decltype(auto) largest_unsigned_v = largest_unsigned<T...>::value;

    /** Store the size of the largest floating point type in a range of types in `value`. */
    template <typename... T>
    struct largest_float {
        static constexpr size_t value = max((std::is_floating_point_v<T> ? sizeof(T) : 0)...);
    };

    /** Return the size of the largest floating point type in a range of types. */
    template <typename... T>
    constexpr decltype(auto) largest_float_v = largest_float<T...>::value;

    /** Use `next_up<T>` to obtain a type that is similar to `T` but twice the size. */
    template <typename T>
    class next_up {
    private:
        using BT = std::decay_t<T>;

    public:
        using type = std::conditional_t<std::is_floating_point_v<BT>,
                                        float_by_size_t<2 * sizeof(BT)>,
                                        std::conditional_t<std::is_signed_v<BT>,
                                                           signed_by_size_t<2 * sizeof(BT)>,
                                                           unsigned_by_size_t<2 * sizeof(BT)>>>;
    };

    /** Use `next_up_t<T>` to obtain a type that is similar to `T` but twice the size. */
    template <typename T>
    using next_up_t = typename next_up<T>::type;


    /** `fit_all` is a `std::common_type` replacement that combines signed and unsigned properly.
     *
     *  Combining an unsigned type and signed type sized identical or smaller to a common type will result in an
     *  unsigned type equal in size of the input unsigned, breaking any negative values. `fit_all` will instead return
     *  a signed type that can hold all types.
     */
    template <typename... T>
    class fit_all {
    private:
        static constexpr size_t ls = largest_signed_v<T...>;
        static constexpr size_t lu = largest_unsigned_v<T...>;
        static constexpr size_t lf = largest_float_v<T...>;

    public:
        using type = std::conditional_t<(lf > 0),
                                        float_by_size_t<lf>,
                                        std::conditional_t<(ls > 0),
                                                           std::conditional_t<(ls > lu),
                                                                              signed_by_size_t<ls>,
                                                                              signed_by_size_t<2 * lu>>,
                                                           std::conditional_t<(lu > 0),
                                                                              unsigned_by_size_t<lu>,
                                                                              void>>>;
    };

    /** `fit_all_t` is a `std::common_type_t` replacement that combines signed and unsigned properly.
     *
     *  Combining an unsigned type and signed type sized identical or smaller to a common type will result in an
     *  unsigned type equal in size of the input unsigned, breaking any negative values. `fit_all_t` will instead return
     *  a signed type that can hold all types.
     */
    template <typename... T>
    using fit_all_t = typename fit_all<T...>::type;
} // namespace arithmetic_type_tools
