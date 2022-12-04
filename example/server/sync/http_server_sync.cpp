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
// Example: HTTP server, synchronous
//
//------------------------------------------------------------------------------

#include <boost/asio/ip/tcp.hpp>
#include <boost/http_io.hpp>
#include <boost/http_proto.hpp>
#include <boost/http_proto/detail/number_string.hpp>

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

//------------------------------------------------

// Return a reasonable mime type based on the extension of a file.
string_view
mime_type(string_view path) noexcept
{
    using http_proto::bnf::iequals;
    auto const ext = [&path]
    {
        auto const pos = path.rfind(".");
        if(pos == string_view::npos)
            return string_view{};
        return path.substr(pos);
    }();
    if(iequals(ext, ".htm"))  return "text/html";
    if(iequals(ext, ".html")) return "text/html";
    if(iequals(ext, ".php"))  return "text/html";
    if(iequals(ext, ".css"))  return "text/css";
    if(iequals(ext, ".txt"))  return "text/plain";
    if(iequals(ext, ".js"))   return "application/javascript";
    if(iequals(ext, ".json")) return "application/json";
    if(iequals(ext, ".xml"))  return "application/xml";
    if(iequals(ext, ".swf"))  return "application/x-shockwave-flash";
    if(iequals(ext, ".flv"))  return "video/x-flv";
    if(iequals(ext, ".png"))  return "image/png";
    if(iequals(ext, ".jpe"))  return "image/jpeg";
    if(iequals(ext, ".jpeg")) return "image/jpeg";
    if(iequals(ext, ".jpg"))  return "image/jpeg";
    if(iequals(ext, ".gif"))  return "image/gif";
    if(iequals(ext, ".bmp"))  return "image/bmp";
    if(iequals(ext, ".ico"))  return "image/vnd.microsoft.icon";
    if(iequals(ext, ".tiff")) return "image/tiff";
    if(iequals(ext, ".tif"))  return "image/tiff";
    if(iequals(ext, ".svg"))  return "image/svg+xml";
    if(iequals(ext, ".svgz")) return "image/svg+xml";
    return "application/text";
}

// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
std::string
path_cat(
    string_view base,
    string_view path)
{
	if (base.empty())
		return std::string(path);
    std::string result(base);
#ifdef BOOST_MSVC
    char constexpr path_separator = '\\';
    if(result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
    for(auto& c : result)
        if(c == '/')
            c = path_separator;
#else
    char constexpr path_separator = '/';
    if(result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
#endif
    return result;
}


// This function produces an HTTP response for the given request.
void
handle_request(
    http_proto::serializer& sr,
    http_proto::response& res,
    http_proto::request_view req,
    http_proto::context& ctx)
{
    // Bad Request
    auto const bad_request =
    [&](string_view why)
    {
        res.set_result(
            http_proto::status::bad_request,
            req.version());
        res.fields.append( http_proto::field::server, "Boost.HTTP-Proto" );
        res.fields.append( http_proto::field::content_type, "text/html" );
        res.fields.append( http_proto::field::content_length,
            http_proto::detail::number_string(why.size()) );
        //res.keep_alive(req.keep_alive());
        //res.body() = std::string(why);
        //res.prepare_payload();
        sr.set_header(res);
        sr.set_body(why.data(), why.size());
    };

    // Make sure we can handle the method
#if 0
    if( req.method() != http_proto::method::get &&
        req.method() != http_proto::method::head)
#endif
        return bad_request("Unknown HTTP-method");

#if 0
    // Returns a not found response
    auto const not_found =
    [&req](beast::string_view target)
    {
        http_proto::response<http_proto::string_body> res{http_proto::status::not_found, req.version()};
        res.set(http_proto::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http_proto::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = "The resource '" + std::string(target) + "' was not found.";
        res.prepare_payload();
        return res;
    };

    // Returns a server error response
    auto const server_error =
    [&req](beast::string_view what)
    {
        http_proto::response<http_proto::string_body> res{http_proto::status::internal_server_error, req.version()};
        res.set(http_proto::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http_proto::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = "An error occurred: '" + std::string(what) + "'";
        res.prepare_payload();
        return res;
    };

    // Make sure we can handle the method
    if( req.method() != http_proto::verb::get &&
        req.method() != http_proto::verb::head)
        return send(bad_request("Unknown HTTP-method"));

    // Request path must be absolute and not contain "..".
    if( req.target().empty() ||
        req.target()[0] != '/' ||
        req.target().find("..") != beast::string_view::npos)
        return send(bad_request("Illegal request-target"));

    // Build the path to the requested file
    std::string path = path_cat(doc_root_, req.target());
    if(req.target().back() == '/')
        path.append("index.html");

    // Attempt to open the file
    beast::error_code ec;
    http_proto::file_body::value_type body;
    body.open(path.c_str(), beast::file_mode::scan, ec);

    // Handle the case where the file doesn't exist
    if(ec == beast::errc::no_such_file_or_directory)
        return send(not_found(req.target()));

    // Handle an unknown error
    if(ec)
        return send(server_error(ec.message()));

    // Cache the size since we need it after the move
    auto const size = body.size();

    // Respond to HEAD request
    if(req.method() == http_proto::verb::head)
    {
        http_proto::response<http_proto::empty_body> res{http_proto::status::ok, req.version()};
        res.set(http_proto::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http_proto::field::content_type, mime_type(path));
        res.content_length(size);
        res.keep_alive(req.keep_alive());
        return send(std::move(res));
    }

    // Respond to GET request
    http_proto::response<http_proto::file_body> res{
        std::piecewise_construct,
        std::make_tuple(std::move(body)),
        std::make_tuple(http_proto::status::ok, req.version())};
    res.set(http_proto::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http_proto::field::content_type, mime_type(path));
    res.content_length(size);
    res.keep_alive(req.keep_alive());
    return send(std::move(res));
#endif
}

//------------------------------------------------

class connection
{
    tcp::socket sock_;
    string_view doc_root_;
    http_proto::context& ctx_;
    http_proto::request_parser rp_;
    http_proto::serializer sr_;
    http_proto::response res_;
    std::string s_;
    //http_io::file_body f_;
public:
    connection(
        tcp::socket sock,
        string_view doc_root,
        http_proto::context& ctx)
        : sock_(std::move(sock))
        , doc_root_(doc_root)
        , ctx_(ctx)
        , rp_(ctx)
        , sr_(ctx)
    {
    }

    //--------------------------------------------

    // Report a failure
    void
    fail(http_proto::error_code ec, char const* what)
    {
        std::cerr << what << ": " << ec.message() << "\n";
    }

    //--------------------------------------------

    // Handles an HTTP server connection
    void
    run()
    {
        bool close = false;
        http_io::error_code ec;

        for(;;)
        {
            http_io::read_header(sock_, rp_, ec);
            if(ec)
                break;
            http_io::read_body(sock_, rp_, ec);
            if(ec)
                break;
            //...
        }
        if(ec == http_proto::error::end_of_message)
        {
            handle_request(sr_, res_, rp_.get(), ctx_);
            http_io::write(sock_, sr_, ec);
        }
        else
        {
            std::cout << ec.message() << std::endl;
        }
    #if 0
        for(;;)
        {
            // Read a request
            http_proto::request<http_proto::string_body> req;
            http_proto::read(socket, buffer, req, ec);
            if(ec == http_proto::error::end_of_stream)
                break;
            if(ec)
                return fail(ec, "read");

            // Send the response
            handle_request(*doc_root_, std::move(req), lambda);
            if(ec)
                return fail(ec, "write");
            if(close)
            {
                // This means we should close the connection, usually because
                // the response indicated the "Connection: close" semantic.
                break;
            }
        }
    #endif

        // Send a TCP shutdown
        sock_.shutdown(tcp::socket::shutdown_send, ec);

        // At this point the connection is closed gracefully
    }
};

//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    try
    {
        // Check command line arguments.
        if (argc != 4)
        {
            std::cerr <<
                "Usage: http-server-sync <address> <port> <doc_root>\n" <<
                "Example:\n" <<
                "    http-server-sync 0.0.0.0 8080 .\n";
            return EXIT_FAILURE;
        }
        auto const address = net::ip::make_address(argv[1]);
        auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
        auto const doc_root = std::make_shared<std::string>(argv[3]);

        // The io_context is required for all I/O
        net::io_context ioc(1);

        // context is required for all HTTP protocol operations
        http_proto::context ctx;

        // The acceptor receives incoming connections
        tcp::acceptor acceptor(ioc, {address, port});
        for(;;)
        {
            // This will receive the new connection
            tcp::socket sock(ioc);

            // Block until we get a connection
            acceptor.accept(sock);

            // Launch the session, transferring ownership of the socket
            std::thread(
                [&ctx, doc_root, &sock]
                {
                    connection c(
                        std::move(sock),
                        *doc_root,
                        ctx);
                    c.run();
                }).detach();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}

// VFALCO FOR NOW
#include <boost/http_proto/src.hpp>
