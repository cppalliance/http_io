//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_io
//

#ifndef BOOST_HTTP_IO_SRC_HPP
#define BOOST_HTTP_IO_SRC_HPP

/*

This file is meant to be included once,
in a translation unit of the program.

*/

#ifndef BOOST_HTTP_IO_SOURCE
#define BOOST_HTTP_IO_SOURCE
#endif

#include <boost/http_io/detail/impl/except.ipp>

// We include this in case someone is
// using src.hpp as their main header file
#include <boost/http_io.hpp>

#endif
