//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppaliance/beast2
//

#ifndef BOOST_BEAST2_TEST_IMPL_ERROR_HPP
#define BOOST_BEAST2_TEST_IMPL_ERROR_HPP

#include <system_error>
#include <type_traits>

namespace std {
template<>
struct is_error_code_enum<
    ::boost::beast2::test::error>
    : std::true_type {};
} // std

namespace boost {
namespace beast2 {
namespace test {

namespace detail {

class error_cat_type :
    public std::error_category
{
public:
    const char*
    name() const noexcept override
    {
        return "boost.beast2.test";
    }

    std::string
    message(int ev) const override
    {
        switch(static_cast<error>(ev))
        {
        default:
        case error::test_failure: return
            "An automatic unit test failure occurred";
        }
    }
};

} // detail

inline
std::error_code
make_error_code(error e) noexcept
{
    static detail::error_cat_type const cat{};
    return {static_cast<
        std::underlying_type<error>::type>(e), cat };
}

} // test
} // beast2
} // boost

#endif
