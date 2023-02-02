//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_io
//

// Test that header file is self-contained.
#include <boost/buffers/const_buffer.hpp>

#include <boost/asio/buffer.hpp>
#include <boost/static_assert.hpp>

#include "test_suite.hpp"

namespace boost {
namespace http_io {

struct buffers_test
{
#if 0
    using D = decltype(asio::dynamic_buffer(
        std::declval<std::string&>()));
    using CB = D::const_buffers_type;
    using MB = D::mutable_buffers_type;

    BOOST_STATIC_ASSERT(
        http_proto::is_const_buffers<CB>::value);
    BOOST_STATIC_ASSERT(
        http_proto::is_const_buffers<MB>::value);
    BOOST_STATIC_ASSERT(
        http_proto::is_mutable_buffers<MB>::value);
#endif

    void
    run()
    {
    }
};

TEST_SUITE(
    buffers_test,
    "boost.http_io.buffers");

} // http_io
} // boost
