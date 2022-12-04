//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

//------------------------------------------------------------------------------
//
// Example: HTTP client, synchronous
//
//------------------------------------------------------------------------------

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/http_io.hpp>
#include <boost/http_proto.hpp>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

namespace http_io = boost::http_io;
namespace http_proto = boost::http_proto;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;
using string_view = http_proto::string_view;

// Performs an HTTP GET and prints the response
int main(int argc, char** argv)
{
    try
    {
        // Check command line arguments.
        if(argc != 4 && argc != 5)
        {
            std::cerr <<
                "Usage: http-client-sync <host> <port> <target> [<HTTP version: 1.0 or 1.1(default)>]\n" <<
                "Example:\n" <<
                "    http-client-sync www.example.com 80 /\n" <<
                "    http-client-sync www.example.com 80 / 1.0\n";
            return EXIT_FAILURE;
        }
        auto const host = argv[1];
        auto const port = argv[2];
        auto const target = argv[3];
        int version = argc == 5 &&
            !std::strcmp("1.0", argv[4]) ? 10 : 11;

        // The io_context is required for all I/O
        net::io_context ioc;

        // These objects perform our I/O
        tcp::resolver resolver(ioc);
        tcp::socket sock(ioc.get_executor());

        // Look up the domain name
        auto const results = resolver.resolve(host, port);

        // Make the connection on the IP address we get from a lookup
        net::connect(sock, results);

        // Set up an HTTP GET request message
        http_proto::request req;
        req.set_start_line(
            http_proto::method::get,
            target,
            http_proto::version::http_1_1 );
        req.emplace_back(
            http_proto::field::host, host );
        req.emplace_back(
            http_proto::field::user_agent, "Boost.HTTP-Proto" );

        http_proto::error_code ec;

        // Send the HTTP request to the remote host
        http_proto::context ctx;
        http_proto::serializer sr;
        sr.reset(req);
        //sr.set_body(nullptr, 0);
        http_io::write( sock, sr, ec );

        // Receive the HTTP response
        http_proto::response_parser p( ctx );
        http_io::read_header( sock, p, ec );

        // Write the message to standard out
        std::cout << ec.message() << std::endl;

        // Gracefully close the socket
        sock.shutdown(tcp::socket::shutdown_both, ec);

        // not_connected happens sometimes
        // so don't bother reporting it.
        //
        /*
        if(ec && ec != beast::errc::not_connected)
            throw beast::system_error{ec};
        */

        // If we get here then the connection is closed gracefully
    }
    catch(std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

//]

// VFALCO FOR NOW
#include <boost/http_proto/src.hpp>
