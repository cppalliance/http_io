//
// Copyright (c) 2022 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_io
//

#include <boost/asio.hpp>
#include <boost/http_io.hpp>
#include <boost/http_proto.hpp>
#include <boost/url.hpp>
#include <boost/core/detail/string_view.hpp>
#include <functional>
#include <iostream>
#include <vector>

#define LOGGING

namespace server {

namespace io = boost::http_io;
namespace urls = boost::urls;
namespace asio = boost::asio;
namespace core = boost::core;
namespace system = boost::system;
namespace http_proto = boost::http_proto;
using namespace std::placeholders;
using tcp = boost::asio::ip::tcp;

// Return a reasonable mime type based on the extension of a file.
core::string_view
get_extension(
    core::string_view path) noexcept
{
    auto const pos = path.rfind(".");
    if(pos == core::string_view::npos)
        return core::string_view();
    return path.substr(pos);
}

core::string_view
mime_type(
    core::string_view path)
{
    using urls::grammar::ci_is_equal;
    auto ext = get_extension(path);
    if(ci_is_equal(ext, ".htm"))  return "text/html";
    if(ci_is_equal(ext, ".html")) return "text/html";
    if(ci_is_equal(ext, ".php"))  return "text/html";
    if(ci_is_equal(ext, ".css"))  return "text/css";
    if(ci_is_equal(ext, ".txt"))  return "text/plain";
    if(ci_is_equal(ext, ".js"))   return "application/javascript";
    if(ci_is_equal(ext, ".json")) return "application/json";
    if(ci_is_equal(ext, ".xml"))  return "application/xml";
    if(ci_is_equal(ext, ".swf"))  return "application/x-shockwave-flash";
    if(ci_is_equal(ext, ".flv"))  return "video/x-flv";
    if(ci_is_equal(ext, ".png"))  return "image/png";
    if(ci_is_equal(ext, ".jpe"))  return "image/jpeg";
    if(ci_is_equal(ext, ".jpeg")) return "image/jpeg";
    if(ci_is_equal(ext, ".jpg"))  return "image/jpeg";
    if(ci_is_equal(ext, ".gif"))  return "image/gif";
    if(ci_is_equal(ext, ".bmp"))  return "image/bmp";
    if(ci_is_equal(ext, ".ico"))  return "image/vnd.microsoft.icon";
    if(ci_is_equal(ext, ".tiff")) return "image/tiff";
    if(ci_is_equal(ext, ".tif"))  return "image/tiff";
    if(ci_is_equal(ext, ".svg"))  return "image/svg+xml";
    if(ci_is_equal(ext, ".svgz")) return "image/svg+xml";
    return "application/text";
}

// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
void
path_cat(
    std::string& result,
    core::string_view base,
    core::string_view path)
{
    if(base.empty())
    {
        result = path;
        return;
    }
    result = base;

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
}

//------------------------------------------------

class file_handler
{
    std::string doc_root_;

public:
    file_handler(
        core::string_view doc_root)
        : doc_root_(doc_root)
    {
    }

    void
    operator()(
        http_proto::request_view const& req,
        http_proto::response& res,
        http_proto::serializer& sr) const
    {
        (void)req;
        (void)res;
        (void)sr;
    }

private:
};

//------------------------------------------------

void
make_error_response(
    http_proto::status code,
    http_proto::request_view const& req,
    http_proto::response& res,
    http_proto::serializer& sr)
{
    auto rv = urls::parse_authority(
        req.value_or(http_proto::field::host, ""));
    core::string_view host;
    if(rv.has_value())
        host = rv->buffer();
    else
        host = "";

    std::string s;
    s  = "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n";
    s += "<html><head>\n";
    s += "<title>";
        s += std::to_string(static_cast<
            std::underlying_type<
                http_proto::status>::type>(code));
        s += " ";
        s += http_proto::obsolete_reason(code);
        s += "</title>\n";
    s += "</head><body>\n";
    s += "<h1>";
        s += http_proto::obsolete_reason(code);
        s += "</h1>\n";
    if(code == http_proto::status::not_found)
    {
        s += "<p>The requested URL ";
        s += req.target_text();
        s += " was not found on this server.</p>\n";
    }
    s += "<hr>\n";
    s += "<address>Boost.Http.IO/1.0b (Win10) Server at ";
        s += rv->host_address();
        s += " Port ";
        s += rv->port();
        s += "</address>\n";
    s += "</body></html>\n";

    res.set_start_line(code, res.version());
    res.set_keep_alive(req.keep_alive());
    res.set_payload_size(s.size());
    res.append(http_proto::field::content_type,
        "text/html; charset=iso-8859-1");
    res.append(http_proto::field::date,
        "Mon, 12 Dec 2022 03:26:32 GMT");
    res.append(http_proto::field::server,
        "Boost.Http.IO/1.0b (Win10)");

    sr.start(
        res,
        http_proto::string_body(
            std::move(s)));
}

//------------------------------------------------

void
handle_request(
    core::string_view doc_root,
    http_proto::request_view const& req,
    http_proto::response& res,
    http_proto::serializer& sr)
{
#if 0
    // Returns a server error response
    auto const server_error =
    [&req](beast::string_view what)
    {
        http::response<http::string_body> res{http::status::internal_server_error, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = "An error occurred: '" + std::string(what) + "'";
        res.prepare_payload();
        return res;
    };
#endif

    // Request path must be absolute and not contain "..".
    if( req.target_text().empty() ||
        req.target_text()[0] != '/' ||
        req.target_text().find("..") != core::string_view::npos)
        return make_error_response(
            http_proto::status::bad_request, req, res, sr);

    // Build the path to the requested file
    std::string path;
    path_cat(path, doc_root, req.target_text());
    if(req.target_text().back() == '/')
        path.append("index.html");

    // Attempt to open the file
    system::error_code ec;
    http_proto::file f;
    std::uint64_t size = 0;
    f.open(path.c_str(), http_proto::file_mode::scan, ec);
    if(! ec.failed())
        size = f.size(ec);
    if(! ec.failed())
    {
        res.set_start_line(
            http_proto::status::ok,
            req.version());
        res.set(http_proto::field::server, "Boost");
        res.set_keep_alive(req.keep_alive());
        res.set_payload_size(size);

        auto mt = mime_type(get_extension(path));
        res.append(
            http_proto::field::content_type, mt);

        sr.start(
            res,
            http_proto::file_body(
                std::move(f), size));
        return;
    }

    // ec.message()?
    return make_error_response(
        http_proto::status::internal_server_error,
            req, res, sr);
}

//------------------------------------------------

BOOST_STATIC_ASSERT(
    std::is_move_constructible<http_proto::serializer>::value);

template< class Executor >
class worker
{
    // order of destruction matters here
    std::string const& doc_root_;
    http_proto::request_parser pr_;
    http_proto::response res_;
    http_proto::serializer sr_;
    asio::basic_socket_acceptor<tcp, Executor>& a_;
    asio::basic_stream_socket<tcp, Executor> s_;
    int id_ = 0;

public:
    worker(worker&&) = default;
    worker(worker const&) = delete;

    worker(
        http_proto::context& ctx,
        std::string const& doc_root,
        asio::basic_socket_acceptor<
            tcp, Executor>& a)
        : doc_root_(doc_root)
        , pr_(ctx)
        , sr_(65536)
        , a_(a)
        , s_(a_.get_executor())
        , id_([]
            {
                static int id = 0;
                return ++id;
            }())
    {
    }

    void
    run()
    {
        accept();
    }

private:
    void
    fail(
        std::string what,
        system::error_code ec)
    {
        if( ec == asio::error::operation_aborted )
            return;

        if( ec == asio::error::eof )
        {
            s_.shutdown(
                asio::socket_base::shutdown_send, ec);
            return;
        }

    #ifdef LOGGING
        std::cerr <<
            what << "[" << id_ << "]: " <<
            ec.message() << "\n";
    #endif
    }

    void
    accept()
    {
        // Clean up any previous connection.
        io::error_code ec;
        s_.close(ec);
        pr_.reset();

        a_.async_accept( s_,
            [this](io::error_code const& ec)
            {
                if( ec.failed() )
                {
                    fail("async_accept", ec);
                    if( ec == asio::error::operation_aborted )
                        return;
                    return accept();
                }

                // Request must be fully processed within 60 seconds.
                //request_deadline_.expires_after(
                    //std::chrono::seconds(60));

                do_read();
            });
    }

    void
    do_read()
    {
        pr_.start();

        io::async_read_header(s_, pr_, std::bind(
            &worker::on_read_header, this, _1, _2));
    }

    void
    on_read_header(
        system::error_code ec,
        std::size_t bytes_transferred)
    {
        (void)bytes_transferred;

        if(ec.failed())
        {
            fail("async_read_header", ec);
            if(ec == asio::error::operation_aborted)
                return;
            return accept();
        }

        io::async_read(s_, pr_, std::bind(
            &worker::on_read_body, this, _1, _2));
    }

    void
    on_read_body(
        system::error_code ec,
        std::size_t bytes_transferred)
    {
        (void)bytes_transferred;

        if( ec.failed() )
        {
            fail("async_read", ec);
            if( ec == asio::error::operation_aborted )
                return;
            return accept();
        }

        res_.clear();

        handle_request(
            doc_root_,
            pr_.get(),
            res_,
            sr_);

    #ifdef LOGGING
        std::cerr << 
            pr_.get().buffer() <<
            res_.buffer() <<
            "--------------------------------------------------\n";
    #endif

        io::async_write(s_, sr_, std::bind(
            &worker::on_write, this, _1, _2));
    }

    void
    on_write(
        system::error_code ec,
        std::size_t bytes_transferred)
    {
        (void)bytes_transferred;

        if( ec.failed() )
        {
            fail("async_write", ec);
            if( ec == asio::error::operation_aborted )
                return;
            return accept();
        }

        do_read();
    }
};

//------------------------------------------------

int main(int argc, char* argv[])
{
    try
    {
        // Check command line arguments.
        if (argc != 5)
        {
            std::cerr << "Usage: http_server_async <address> <port> <doc_root> <num_workers>\n";
            std::cerr << "  For IPv4, try:\n";
            std::cerr << "    http_server_async 0.0.0.0 80 . 100\n";
            std::cerr << "  For IPv6, try:\n";
            std::cerr << "    http_server_async 0::0 80 . 100\n";
            return EXIT_FAILURE;
        }

        auto const addr = asio::ip::make_address(argv[1]);
        unsigned short const port = static_cast<unsigned short>(std::atoi(argv[2]));
        std::string const doc_root = argv[3];
        int const num_workers = std::atoi(argv[4]);

        using executor_type = asio::io_context::executor_type;

        asio::io_context ioc( 1 );
        asio::basic_socket_acceptor<tcp, executor_type> a( ioc, { addr, port } );

        file_handler fh(doc_root);

        // Capture SIGINT and SIGTERM to perform a clean shutdown
        asio::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait(
            [&](system::error_code const&, int)
            {
                // Stop the `io_context`. This will cause `run()`
                // to return immediately, eventually destroying the
                // `io_context` and all of the sockets in it.
                ioc.stop();
            });

        http_proto::context ctx;
        http_proto::request_parser::config cfg;
        http_proto::install_parser_service(ctx, cfg);

        std::vector<worker<executor_type>> v;
        v.reserve( num_workers );
        for(auto i = num_workers; i--;)
        {
            v.emplace_back( ctx, doc_root, a );
            v.back().run();
        }
        ioc.run();
    }
    catch( std::exception const& e )
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }    
    return EXIT_SUCCESS;
}

} // server

int main(int argc, char* argv[])
{
    return server::main(argc, argv);
}
