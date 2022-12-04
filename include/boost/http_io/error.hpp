//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_io
//

#ifndef BOOST_HTTP_IO_ERROR_HPP
#define BOOST_HTTP_IO_ERROR_HPP

#include <boost/http_io/detail/config.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

#include <boost/url/error_types.hpp>

namespace boost {
namespace http_io {

using namespace urls::error_types;

} // http_io
} // boost

#endif
