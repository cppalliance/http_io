//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_io
//

#ifndef BOOST_HTTP_IO_IMPL_READ_HPP
#define BOOST_HTTP_IO_IMPL_READ_HPP

#include <boost/http_io/buffer.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/parser.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/compose.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/assert.hpp>

namespace boost {
namespace http_io {

namespace detail {

class read_buffers
{
    http_proto::parser::buffers b_;

public:
    class iterator;

    explicit
    read_buffers(
        http_proto::parser::buffers b) noexcept
        : b_(b)
    {
    }

    iterator begin() const noexcept;
    iterator end() const noexcept;
};

class read_buffers::iterator
{
    using buffers_type =
        http_proto::parser::buffers;
    using iter_type = buffers_type::iterator;

    iter_type it_{};

    friend class read_buffers;

    iterator(iter_type it)
        : it_(it)
    {
    }

public:
    using value_type = asio::mutable_buffer;
    using reference = value_type;
    using pointer = value_type const*;
    using difference_type = std::ptrdiff_t;
    using iterator_category =
        std::forward_iterator_tag;

    iterator() = default;
    iterator(
        iterator const&) = default;
    iterator& operator=(
        iterator const&) = default;

    asio::mutable_buffer
    operator*() const noexcept
    {
        auto const mb(*it_);
        return { mb.data(), mb.size() };
    }

    bool
    operator==(
        iterator const& other) const noexcept
    {
        return it_ == other.it_;
    }

    bool
    operator!=(
        iterator const& other) const noexcept
    {
        return it_ != other.it_;
    }

    iterator&
    operator++() noexcept
    {
        ++it_;
        return *this;
    }

    iterator
    operator++(int) noexcept
    {
        auto temp = *this;
        ++(*this);
        return temp;
    }
};

inline
auto
read_buffers::
begin() const noexcept ->
    iterator
{
    return iterator(b_.begin());
}

inline
auto
read_buffers::
end() const noexcept ->
    iterator
{
    return iterator(b_.end());
}

//------------------------------------------------

template<class Stream>
class read_some_op
    : public asio::coroutine
{
    Stream& s_;
    http_proto::parser& p_;

public:
    read_some_op(
        Stream& s,
        http_proto::parser& p) noexcept
        : s_(s)
        , p_(p)
    {
        BOOST_ASSERT(! p_.is_complete());
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
            BOOST_ASIO_CORO_YIELD
            {
                BOOST_ASIO_HANDLER_LOCATION((
                    __FILE__, __LINE__,
                    "Stream::async_read_some"));
                s_.async_read_some(
                    read_buffers(
                        p_.prepare()),
                    std::move(self));
            }
            if(! ec.failed())
            {
                p_.commit(bytes_transferred, ec);
            }
            else if(ec == asio::error::eof)
            {
                BOOST_ASSERT(
                    bytes_transferred == 0);
                p_.commit_eof(ec);
            }

            self.complete(ec, bytes_transferred);
        }
    }
};

//------------------------------------------------

template<class Stream>
class read_op
    : public asio::coroutine
{
    Stream& s_;
    http_proto::parser& p_;
    std::size_t n_ = 0;

public:
    read_op(
        Stream& s,
        http_proto::parser& p) noexcept
        : s_(s)
        , p_(p)
    {
        BOOST_ASSERT(! p_.is_complete());
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
            do
            {
                BOOST_ASIO_CORO_YIELD
                {
                    BOOST_ASIO_HANDLER_LOCATION((
                        __FILE__, __LINE__,
                        "http_io::async_read_some"));
                    http_io::async_read_some(
                        s_, p_, std::move(self));
                }
                n_ += bytes_transferred;
                if(ec.failed())
                    break;
            }
            while(! p_.is_complete());

            self.complete(ec, n_);
        }
    }
};

} // detail

//------------------------------------------------

template<
    class AsyncReadStream,
    BOOST_ASIO_COMPLETION_TOKEN_FOR(
        void(error_code, std::size_t)) CompletionToken>
BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken,
    void (error_code, std::size_t))
async_read_some(
    AsyncReadStream& s,
    boost::http_proto::parser& p,
    CompletionToken&& token)
{
    return asio::async_compose<
        CompletionToken,
        void(error_code, std::size_t)>(
            detail::read_some_op<
                AsyncReadStream>{s, p},
            token,
            s);
}

template<
    class AsyncReadStream,
    BOOST_ASIO_COMPLETION_TOKEN_FOR(
        void(error_code, std::size_t)) CompletionToken>
BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken,
    void (error_code, std::size_t))
async_read(
    AsyncReadStream& s,
    boost::http_proto::parser& p,
    CompletionToken&& token)
{
    return asio::async_compose<
        CompletionToken,
        void(error_code, std::size_t)>(
            detail::read_op<
                AsyncReadStream>{s, p},
            token,
            s);
}

} // http_io
} // boost

#endif
