//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_io
//

#ifndef BOOST_HTTP_IO_READ_HPP
#define BOOST_HTTP_IO_READ_HPP

#include <boost/http_io/detail/config.hpp>
#include <boost/http_io/error.hpp>
#include <boost/http_proto/parser.hpp>
#include <boost/asio/async_result.hpp>
#include <cstdint>

namespace boost {
namespace http_io {

/** Read from a stream into a parser
*/
template<
    class AsyncReadStream,
    BOOST_ASIO_COMPLETION_TOKEN_FOR(
        void(error_code, std::size_t)) CompletionToken
            BOOST_ASIO_DEFAULT_COMPLETION_TOKEN_TYPE(
                typename AsyncReadStream::executor_type)>
BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken,
    void (error_code, std::size_t))
async_read_some(
    AsyncReadStream& s,
    boost::http_proto::parser& p,
    CompletionToken&& token
        BOOST_ASIO_DEFAULT_COMPLETION_TOKEN(
            typename AsyncReadStream::executor_type));

/** Read a complete HTTP message
*/
template<
    class AsyncReadStream,
    BOOST_ASIO_COMPLETION_TOKEN_FOR(
        void(error_code, std::size_t)) CompletionToken>
BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken,
    void (error_code, std::size_t))
async_read(
    AsyncReadStream& s,
    boost::http_proto::parser& p,
    CompletionToken&& token);

} // http_io
} // boost

#include <boost/http_io/impl/read.hpp>

#endif
