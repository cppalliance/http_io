//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_io
//

#ifndef BOOST_HTTP_IO_IMPL_WRITE_HPP
#define BOOST_HTTP_IO_IMPL_WRITE_HPP

#include <boost/http_io/buffer.hpp>
#include <boost/http_io/error.hpp>
#include <boost/asio/compose.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/asio/post.hpp>
#include <vector>

namespace boost {
namespace http_io {

namespace detail {

template<
    class Stream,
    class Allocator>
class write_some_op
    : public asio::coroutine
{
    Stream& s_;
    std::vector<
        asio::const_buffer,
        Allocator> v_;
    http_proto::serializer& sr_;

public:
    write_some_op(
        Stream& s,
        Allocator const& a,
        http_proto::serializer& sr) noexcept
        : s_(s)
        , v_(a)
        , sr_(sr)
    {
    }

    template<class Self>
    void
    operator()(
        Self& self,
        error_code ec = {},
        std::size_t bytes_transferred = 0)
    {
        urls::result<http_proto::const_buffers> rv;
        BOOST_ASIO_CORO_REENTER(*this)
        {
            // Calling this function when the
            // serializer has nothing to do
            // incurs an unnecessary cost.
            // 
            BOOST_ASSERT(! sr_.is_complete());

            rv = sr_.prepare();
            if(! rv)
            {
                ec = rv.error();
                BOOST_ASIO_CORO_YIELD
                {
                    BOOST_ASIO_HANDLER_LOCATION((
                        __FILE__, __LINE__,
                        "http_io::write_some_op"));
                    asio::post(std::move(self));
                }
                goto upcall;
            }

            BOOST_ASIO_CORO_YIELD
            {
                BOOST_ASIO_HANDLER_LOCATION((
                    __FILE__, __LINE__,
                    "http_io::write_some_op"));
                s_.async_write_some(
                    const_buffers(*rv),
                    std::move(self));
            }
            sr_.consume(bytes_transferred);

        upcall:
            {
                auto v(std::move(v_));
            }
            self.complete(
                ec, bytes_transferred );
        }
    }
};

//------------------------------------------------

template<class Stream>
class write_op
    : public asio::coroutine
{
    Stream& s_;
    http_proto::serializer& sr_;
    std::size_t n_ = 0;

public:
    write_op(
        Stream& s,
        http_proto::serializer& sr) noexcept
        : s_(s)
        , sr_(sr)
    {
    }

    template<class Self>
    void
    operator()(
        Self& self,
        error_code ec = {},
        std::size_t bytes_transferred = 0)
    {
        BOOST_ASIO_CORO_REENTER(*this)
        {
            // Calling this function when the
            // serializer has nothing to do
            // incurs an unnecessary cost.
            // 
            BOOST_ASSERT(! sr_.is_complete());

        #if 0
            if(sr_.is_complete())
            {
                BOOST_ASIO_CORO_YIELD
                {
                    BOOST_ASIO_HANDLER_LOCATION((
                        __FILE__, __LINE__,
                        "http_io::write_op"));
                    asio::post(std::move(self));
                }
                goto upcall;
            }
        #endif

            do
            {
                BOOST_ASIO_CORO_YIELD
                {
                    BOOST_ASIO_HANDLER_LOCATION((
                        __FILE__, __LINE__,
                        "http_io::write_op"));
                    async_write_some(
                        s_, sr_, std::move(self));
                }
                n_ += bytes_transferred;
                if(ec.failed())
                    break;
            }
            while(! sr_.is_complete());

            // upcall
            self.complete(ec, n_ );
        }
    }
};

} // detail

//------------------------------------------------

template<
    class AsyncWriteStream,
    BOOST_ASIO_COMPLETION_TOKEN_FOR(
        void(error_code, std::size_t)) CompletionToken>
BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken,
    void (error_code, std::size_t))
async_write_some(
    AsyncWriteStream& s,
    boost::http_proto::serializer& sr,
    CompletionToken&& token)
{
    using Allocator =
        asio::associated_allocator_t<
            CompletionToken>;

    return asio::async_compose<
        CompletionToken,
        void(error_code, std::size_t)>(
            detail::write_some_op<
                AsyncWriteStream,
                Allocator>{
                s, asio::get_associated_allocator(
                    token), sr},
            token, s);
}

template<
    class AsyncWriteStream,
    BOOST_ASIO_COMPLETION_TOKEN_FOR(
        void(error_code, std::size_t)) CompletionToken>
BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken,
    void (error_code, std::size_t))
async_write(
    AsyncWriteStream& s,
    boost::http_proto::serializer& sr,
    CompletionToken&& token)
{
    return asio::async_compose<
        CompletionToken,
        void(error_code, std::size_t)>(
            detail::write_op<
                AsyncWriteStream>{s, sr},
            token,
            s);
}

} // http_io
} // boost

#endif
