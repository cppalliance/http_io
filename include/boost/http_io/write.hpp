//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_io
//

#ifndef BOOST_HTTP_IO_WRITE_HPP
#define BOOST_HTTP_IO_WRITE_HPP

#include <boost/http_io/detail/config.hpp>
#include <boost/http_io/error.hpp>
#include <boost/http_proto/serializer.hpp>
#include <boost/asio/async_result.hpp>

namespace boost {
namespace http_io {

/**
*/
template<
    class AsyncWriteStream,
    BOOST_ASIO_COMPLETION_TOKEN_FOR(
        void(error_code, std::size_t)) CompletionToken>
BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken,
    void (error_code, std::size_t))
async_write_some(
    AsyncWriteStream& dest,
    boost::http_proto::serializer& sr,
    CompletionToken&& token);

/**
*/
template<
    class AsyncWriteStream,
    BOOST_ASIO_COMPLETION_TOKEN_FOR(
        void(error_code, std::size_t)) CompletionToken>
BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken,
    void (error_code, std::size_t))
async_write(
    AsyncWriteStream& dest,
    boost::http_proto::serializer& sr,
    CompletionToken&& token);

#if 0
/**
*/
template<
    class AsyncWriteStream,
    class AsyncReadStream,
    class CompletionCondition,
    BOOST_ASIO_COMPLETION_TOKEN_FOR(
        void(error_code, std::size_t)) CompletionToken>
BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken,
    void (error_code, std::size_t))
async_relay_some(
    AsyncWriteStream& dest,
    AsyncReadStream& src,
    CompletionCondition const& cond,
    boost::http_proto::serializer& sr,
    CompletionToken&& token);
#endif

} // http_io
} // boost

#include <boost/http_io/impl/write.hpp>

#endif
