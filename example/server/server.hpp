//
// Copyright (c) 2022 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_io
//

#ifndef BOOST_HTTP_IO_EXAMPLE_SERVER_HPP
#define BOOST_HTTP_IO_EXAMPLE_SERVER_HPP

#include <memory>
#include <type_traits>
#include <vector>

class server
{
public:
    class service
    {
    public:
        virtual ~service() = default;
        virtual void run() = 0;
        virtual void stop() = 0;
    };

    template<class Service, class... Args>
    Service&
    make_service(Args&&... args)
    {
        static_assert(
            std::is_convertible<Service*, service*>::value,
            "Type requirements not met.");

        auto p = std::make_unique<Service>(
            std::forward<Args>(args)...);
        auto& svc = *p;
        v_.emplace_back(std::move(p));
        return svc;
    }

    void
    run()
    {
        for(auto& svc : v_)
            svc.run();
    }

    void
    stop()
    {
        for(auto& svc : v_)
            svc.stop();
    }

private:
    std::vector<service> v_;
};

#endif
