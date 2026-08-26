//
// Copyright (c) 2025 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/beast2
//

#ifndef BOOST_BEAST2_ERROR_HPP
#define BOOST_BEAST2_ERROR_HPP

#include <boost/beast2/detail/config.hpp>
#include <system_error>
#include <type_traits>

namespace boost {
namespace beast2 {

/** Error codes
*/
enum class error
{
    success = 0,
};

} // beast2
} // boost

namespace std {
template<>
struct is_error_code_enum<
    ::boost::beast2::error>
    : std::true_type {};
} // std

namespace boost {
namespace beast2 {

namespace detail {
struct BOOST_SYMBOL_VISIBLE
    error_cat_type
    : std::error_category
{
    BOOST_BEAST2_DECL const char* name(
        ) const noexcept override;
    BOOST_BEAST2_DECL std::string message(
        int) const override;
    constexpr error_cat_type() noexcept = default;
};
BOOST_BEAST2_DECL extern error_cat_type error_cat;
} // detail

inline
std::error_code
make_error_code(
    error ev) noexcept
{
    return std::error_code{
        static_cast<std::underlying_type<
            error>::type>(ev),
        detail::error_cat};
}

} // beast2
} // boost

#endif
