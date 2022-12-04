//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_io
//

#ifndef BOOST_HTTP_IO_BUFFER_HPP
#define BOOST_HTTP_IO_BUFFER_HPP

#include <boost/http_io/detail/config.hpp>
#include <boost/http_proto/buffer.hpp>
#include <boost/asio/buffer.hpp>
#include <cstdlib>
#include <iterator>

namespace boost {
namespace http_io {

//------------------------------------------------

class mutable_buffers
{
    http_proto::mutable_buffer const* p_ = nullptr;
    std::size_t n_ = 0;

public:
#ifdef BOOST_IO_DOCS
    using iterator = __implementation_defined__;
#else
    class iterator;
#endif

    mutable_buffers() = default;

    mutable_buffers(
        http_proto::mutable_buffers const& b) noexcept
        : p_(b.begin())
        , n_(b.size())
    {
    }

    iterator
    begin() const noexcept;

    iterator
    end() const noexcept;
};

//------------------------------------------------

class const_buffers
{
    http_proto::const_buffer const* p_ = nullptr;
    std::size_t n_ = 0;

public:
#ifdef BOOST_IO_DOCS
    using iterator = __implementation_defined__;
#else
    class iterator;
#endif

    const_buffers() = default;

    const_buffers(
        http_proto::const_buffers const& b) noexcept
        : p_(b.begin())
        , n_(b.size())
    {
    }

    iterator
    begin() const noexcept;

    iterator
    end() const noexcept;
};

//------------------------------------------------

class mutable_buffers::iterator
{
    friend class mutable_buffers;

    http_proto::mutable_buffer const* p_ = nullptr;

    explicit
    iterator(
        http_proto::mutable_buffer const* p)
        : p_(p)
    {
    }

public:
    using value_type = asio::mutable_buffer;
    using reference = value_type;
    using pointer = value_type const*;
    using difference_type =
        std::ptrdiff_t;
    using iterator_category =
        std::forward_iterator_tag;

    bool
    operator==(
        iterator const& other) const noexcept
    {
        return p_ == other.p_;
    }

    bool
    operator!=(
        iterator const& other) const noexcept
    {
        return !(*this == other);
    }

    reference
    operator*() const noexcept
    {
        return { p_->data(), p_->size() };
    }

    iterator&
    operator++() noexcept
    {
        ++p_;
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

//------------------------------------------------

class const_buffers::iterator
{
    friend class const_buffers;

    http_proto::const_buffer const* p_ = nullptr;

    explicit
    iterator(
        http_proto::const_buffer const* p)
        : p_(p)
    {
    }

public:
    using value_type = asio::const_buffer;
    using reference = value_type;
    using pointer = value_type const*;
    using difference_type =
        std::ptrdiff_t;
    using iterator_category =
        std::forward_iterator_tag;

    bool
    operator==(
        iterator const& other) const noexcept
    {
        return p_ == other.p_;
    }

    bool
    operator!=(
        iterator const& other) const noexcept
    {
        return !(*this == other);
    }

    reference
    operator*() const noexcept
    {
        return { p_->data(), p_->size() };
    }

    iterator&
    operator++() noexcept
    {
        ++p_;
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

//------------------------------------------------

inline
auto
mutable_buffers::
begin() const noexcept ->
    iterator
{
    return iterator(p_);
}

inline
auto
mutable_buffers::
end() const noexcept ->
    iterator
{
    return iterator(p_ + n_);
}

inline
auto
const_buffers::
begin() const noexcept ->
    iterator
{
    return iterator(p_);
}

inline
auto
const_buffers::
end() const noexcept ->
    iterator
{
    return iterator(p_ + n_);
}

} // http_io
} // boost

#endif
