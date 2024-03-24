//
// Copyright (c) 2022 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_io
//

#include "server.hpp"

server::
server()
    : ioc_(1)
    , sigs_(ioc_, SIGINT, SIGTERM)
{
}

auto
server::
make_executor() ->
    executor_type
{
    return ioc_.get_executor();
}

void
server::
run()
{
    // Capture SIGINT and SIGTERM to
    // perform a clean shutdown
    sigs_.async_wait(
        [&](boost::system::error_code const&, int)
        {
            // cancel all outstanding work,
            // causing io_context::run to return.
            this->stop();
        });

    for(auto& svc : v_)
        svc->run();

    ioc_.run();
}

void
server::
stop()
{
    // all outstanding work must be owned by a service.
    for(auto& svc : v_)
        svc->stop();
}
