#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>


namespace arithmetic_type_tools {
    namespace {
        // Why isn't std::max variadic?
        template <typename T>
        constexpr T&& vmax(T&& val) {
            return std::forward<T>(val);
        }

        template <typename T, typename U, typename... V>
        constexpr decltype(auto) vmax(T&& val1, U&& val2, V&&... rest) {
            return (val1 >= val2)
                        ? vmax(val1, std::forward<V>(rest)...)
                        : vmax(val2, std::forward<V>(rest)...);
        }

    } // namespace

    /** Store the size of the largest signed type in a range of types in `value`. */
    template <typename... T>
    struct largest_signed {
        static constexpr size_t value = vmax((std::is_signed_v<T> ? sizeof(T) : 0)...);
    };

    /** Return the size of the largest signed type in a range of types. */
    template <typename... T>
    constexpr decltype(auto) largest_signed_v = largest_signed<T...>::value;

    /** Store the size of the largest unsigned type in a range of types in `value`. */
    template <typename... T>
    struct largest_unsigned {
        static constexpr size_t value = vmax((std::is_unsigned_v<T> ? sizeof(T) : 0)...);
    };

    /** Return the size of the largest signed type in a range of types. */
    template <typename... T>
    constexpr decltype(auto) largest_unsigned_v = largest_unsigned<T...>::value;

    /** Store the size of the largest floating point type in a range of types in `value`. */
    template <typename... T>
    struct largest_float {
        static constexpr size_t value = vmax((std::is_floating_point_v<T> ? sizeof(T) : 0)...);
    };

    /** Return the size of the largest floating point type in a range of types. */
    template <typename... T>
    constexpr decltype(auto) largest_float_v = largest_float<T...>::value;

    /**@{ Allow selecting types based on their size */
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

    /** Select a type similar to `T`, but one size up. */
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

    /** Use `next_up_t<T>` to obtain a type that is similar but twice the size. */
    template <typename T>
    using next_up_t = typename next_up<T>::type;


    /** fit_all is a std::common_type replacement that does combine signed and unsigned properly. */
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

    /** fit_all_t is a std::common_type_t replacement that does combine signed and unsigned properly. */
    template <typename... T>
    using fit_all_t = typename fit_all<T...>::type;

} // namespace arithmetic_type_tools
