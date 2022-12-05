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

#include <boost/http_io/error.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/compose.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/asio/post.hpp>
#include <iterator>

namespace boost {
namespace http_io {

namespace detail {

class write_buffers
{
    http_proto::serializer::buffers b_;

public:
    class iterator;

    explicit
    write_buffers(
        http_proto::serializer::buffers b) noexcept
        : b_(b)
    {
    }

    iterator begin() const noexcept;
    iterator end() const noexcept;
};

class write_buffers::iterator
{
    using iter_type = 
        http_proto::serializer::buffers::iterator;

    iter_type it_{};

    friend class write_buffers;

    iterator(iter_type it)
        : it_(it)
    {
    }

public:
    using value_type = asio::const_buffer;
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

    asio::const_buffer
    operator*() const noexcept
    {
        auto const cb(*it_);
        return { cb.data(), cb.size() };
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
write_buffers::
begin() const noexcept ->
    iterator
{
    return iterator(b_.begin());
}

inline
auto
write_buffers::
end() const noexcept ->
    iterator
{
    return iterator(b_.end());
}

//------------------------------------------------

template<class Stream>
class write_some_op
    : public asio::coroutine
{
    Stream& s_;
    http_proto::serializer& sr_;

public:
    write_some_op(
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
        urls::result<
            http_proto::serializer::buffers> rv;

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
                    write_buffers(*rv),
                    std::move(self));
            }
            sr_.consume(bytes_transferred);

        upcall:
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
    return asio::async_compose<
        CompletionToken,
        void(error_code, std::size_t)>(
            detail::write_some_op<
                AsyncWriteStream>{s, sr},
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
