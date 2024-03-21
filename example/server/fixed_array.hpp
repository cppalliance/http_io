//
// Copyright (c) 2022 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_io
//

#ifndef FIXED_ARRAY_HPP
#define FIXED_ARRAY_HPP

#include <cstdlib>

template<class T>
class fixed_array
{
//#if __cplusplus < 201703L // gcc nonconforming
    static_assert(
        alignof(T) <=
            alignof(std::max_align_t),
        "T must not be overaligned");
//#endif

    T* t_ = nullptr;
    std::size_t n_ = 0;

    fixed_array() = default;

public:
    using value_type = T;
    using reference = T&;
    using pointer = T*;
    using iterator = T*;
    using const_reference = T const&;
    using const_pointer = T const*;
    using const_iterator = T const*;
    using difference_type = std::ptrdiff_t;
    using size_type = std::size_t;

    template<class... Args>
    explicit
    fixed_array(
        std::size_t N,
        Args&&... args)
        : fixed_array()
    {
        t_ = std::allocator<T>{}.allocate(N);
        while(n_ < N)
        {
            ::new(&t_[n_]) T(args...);
            ++n_;
        }
    }

    ~fixed_array()
    {
        if(! t_)
            return;
        for(auto n = n_; n--;)
            t_[n].~T();
        std::allocator<T>{}.deallocate(t_, n_);
    }

    std::size_t
    size() const noexcept
    {
        return n_;
    }

    iterator
    begin() noexcept
    {
        return t_;
    }

    iterator
    end() noexcept
    {
        return t_ + n_;
    }

    const_iterator
    begin() const noexcept
    {
        return const_cast<
            T const*>(t_);
    }

    const_iterator
    end() const noexcept
    {
        return const_cast<
            T const*>(t_) + n_;
    }
};

#endif
