//
// Copyright (c) 2022 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_io
//

#include "server.hpp"
#include <functional>

server::
server()
    : ioc_(1)
    , sigs_(make_executor(), SIGINT, SIGTERM)
    , timer_(make_executor())
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
    using namespace std::placeholders;

    // Capture SIGINT and SIGTERM to
    // perform a clean shutdown
    sigs_.async_wait(std::bind(
        &server::on_signal, this, _1, _2));

    for(auto& svc : v_)
        svc->run();

    ioc_.run();
}

void
server::
stop()
{
    if(is_stopped_)
    {
        // happens when there's a race with
        // the signal and the timer handlers
        return;
    }
    is_stopped_ = true;

    boost::system::error_code ec;
    sigs_.cancel(ec); // VFALCO should we use the 0-arg overload?
    timer_.cancel();

    for(auto& svc : v_)
        svc->stop();
}

void
server::
on_signal(
    boost::system::error_code const& ec, int sig)
{
    using namespace std::placeholders;

    if(! is_shutting_down_)
    {
        // new requests will receive HTTP 503 status
        is_shutting_down_ = true;

        // begin timed, graceful shutdown
        sigs_.async_wait(std::bind(
            &server::on_signal, this, _1, _2));

        timer_.expires_after(std::chrono::seconds(30));
        timer_.async_wait(std::bind(
            &server::on_timer, this, _1));
    }
    else
    {
        // force a stop
        stop();
    }
}

void
server::
on_timer(
    boost::system::error_code const& ec)
{
    if(! ec.failed())
    {
        stop();
    }
    else if(ec != boost::asio::error::operation_aborted)
    {
        // log?
    }
}
