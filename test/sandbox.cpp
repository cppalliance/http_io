//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_io
//

#include "test_suite.hpp"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/http_proto.hpp>
#include <boost/core/detail/string_view.hpp>

namespace boost {
namespace http_proto {

using tcp = asio::ip::tcp;
namespace ssl = asio::ssl;
using string_view =
    boost::core::string_view;

struct sandbox_test
{
    void
    run()
    {
        auto const& log = test_suite::log;
        log << "hello\n";

        asio::io_context ioc;
        tcp::resolver dns(ioc);
        tcp::socket sock(ioc);

        ssl::context ctx(ssl::context::sslv23_client);
        ssl::stream<tcp::socket> tls(ioc, ctx);

        asio::connect(
            sock,
            dns.resolve(
                "httpbin.cpp.al", "http"));

        request req;
        req.set_start_line(
            method::post, "/post", version::http_1_1);
        req.append(field::host, "httpbin.cpp.al");
        req.append(field::accept, "application/text");
        req.append(field::user_agent, "boost");
        req.set_payload_size(1);
        asio::write(sock,
            asio::buffer(req.buffer()));
        asio::write(sock,
            asio::buffer(string_view("*")));

        std::string s;
        s.resize(4096);
        sock.read_some(asio::buffer(s));
        log << s;
    }
};

TEST_SUITE(
    sandbox_test,
    "boost.http_io.sandbox");

} // http_proto
} // boost
