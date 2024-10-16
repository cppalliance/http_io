//
// Copyright (c) 2024 Mohammad Nejati
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_io
//

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/http_io.hpp>
#include <boost/http_proto.hpp>
#include <boost/url.hpp>

#include <cstdlib>
#include <iostream>

#if defined(BOOST_ASIO_HAS_CO_AWAIT)

#include <variant>

namespace asio       = boost::asio;
namespace core       = boost::core;
namespace http_io    = boost::http_io;
namespace http_proto = boost::http_proto;
namespace ssl        = boost::asio::ssl;
namespace urls       = boost::urls;

class any_stream
{
public:
    using executor_type     = asio::any_io_executor;
    using plain_stream_type = asio::ip::tcp::socket;
    using ssl_stream_type   = ssl::stream<plain_stream_type>;

    any_stream(plain_stream_type stream)
        : stream_{ std::move(stream) }
    {
    }

    any_stream(ssl_stream_type stream)
        : stream_{ std::move(stream) }
    {
    }

    executor_type
    get_executor() noexcept
    {
        return std::visit([](auto& s) { return s.get_executor(); }, stream_);
    }

    template<
        typename ConstBufferSequence,
        typename CompletionToken =
            asio::default_completion_token_t<executor_type>>
    auto
    async_write_some(
        const ConstBufferSequence& buffers,
        CompletionToken&& token =
            asio::default_completion_token_t<executor_type>{})
    {
        return boost::asio::async_compose<
            CompletionToken,
            void(boost::system::error_code, size_t)>(
            [this, buffers, init = false](
                auto&& self,
                boost::system::error_code ec = {},
                size_t n                     = 0) mutable
            {
                if(std::exchange(init, true))
                    return self.complete(ec, n);

                std::visit(
                    [&](auto& s)
                    { s.async_write_some(buffers, std::move(self)); },
                    stream_);
            },
            token,
            get_executor());
    }

    template<
        typename MutableBufferSequence,
        typename CompletionToken =
            asio::default_completion_token_t<executor_type>>
    auto
    async_read_some(
        const MutableBufferSequence& buffers,
        CompletionToken&& token =
            asio::default_completion_token_t<executor_type>{})
    {
        return boost::asio::async_compose<
            CompletionToken,
            void(boost::system::error_code, size_t)>(
            [this, buffers, init = false](
                auto&& self,
                boost::system::error_code ec = {},
                size_t n                     = 0) mutable
            {
                if(std::exchange(init, true))
                    return self.complete(ec, n);

                std::visit(
                    [&](auto& s)
                    { s.async_read_some(buffers, std::move(self)); },
                    stream_);
            },
            token,
            get_executor());
    }

    template<
        typename CompletionToken =
            asio::default_completion_token_t<executor_type>>
    auto
    async_shutdown(
        CompletionToken&& token =
            asio::default_completion_token_t<executor_type>{})
    {
        return boost::asio::
            async_compose<CompletionToken, void(boost::system::error_code)>(
                [this, init = false](
                    auto&& self, boost::system::error_code ec = {}) mutable
                {
                    if(std::exchange(init, true))
                        return self.complete(ec);

                    std::visit(
                        [&](auto& s)
                        {
                            if constexpr(std::is_same_v<
                                             decltype(s),
                                             ssl_stream_type&>)
                            {
                                s.async_shutdown(std::move(self));
                            }
                            else
                            {
                                s.close(ec);
                                asio::async_immediate(
                                    s.get_executor(),
                                    asio::append(std::move(self), ec));
                            }
                        },
                        stream_);
                },
                token,
                get_executor());
    }

private:
    std::variant<plain_stream_type, ssl_stream_type> stream_;
};

asio::awaitable<any_stream>
connect(
    ssl::context& ssl_ctx,
    core::string_view host,
    core::string_view service)
{
    auto exec     = co_await asio::this_coro::executor;
    auto resolver = asio::ip::tcp::resolver{ exec };
    auto rresults = co_await resolver.async_resolve(host, service);

    if(service == "https")
    {
        auto stream = ssl::stream<asio::ip::tcp::socket>{ exec, ssl_ctx };
        co_await asio::async_connect(stream.lowest_layer(), rresults);

        if(auto host_s = std::string{ host };
           !SSL_set_tlsext_host_name(stream.native_handle(), host_s.c_str()))
        {
            throw boost::system::system_error(
                static_cast<int>(::ERR_get_error()),
                asio::error::get_ssl_category());
        }

        co_await stream.async_handshake(ssl::stream_base::client);
        co_return stream;
    }

    auto stream = asio::ip::tcp::socket{ exec };
    co_await asio::async_connect(stream, rresults);
    co_return stream;
}

bool
is_redirect(http_proto::response_view resp) noexcept
{
    switch(resp.status())
    {
        case http_proto::status::moved_permanently:
        case http_proto::status::found:
        case http_proto::status::temporary_redirect:
        case http_proto::status::permanent_redirect:
            return true;
        default:
            return false;
    }
}

asio::awaitable<void>
write_get_request(
    http_proto::context& http_proto_ctx,
    any_stream& stream,
    core::string_view host,
    core::string_view target)
{
    auto exec    = co_await asio::this_coro::executor;
    auto request = http_proto::request{};

    request.set_method(http_proto::method::get);
    request.set_target(target);
    request.set_version(http_proto::version::http_1_1);
    request.set_keep_alive(false);
    request.set(http_proto::field::host, host);
    request.set(http_proto::field::user_agent, "Boost.Http.Io");

    #ifdef BOOST_HTTP_PROTO_HAS_ZLIB
    request.set(http_proto::field::accept_encoding, "gzip, deflate");
    #endif

    auto serializer = http_proto::serializer{ http_proto_ctx };
    serializer.start(request);
    co_await http_io::async_write(stream, serializer);
}

// Performs an HTTP GET and prints the response
asio::awaitable<void>
request(
    ssl::context& ssl_ctx,
    http_proto::context& http_proto_ctx,
    core::string_view host,
    core::string_view service,
    core::string_view target)
{
    auto exec   = co_await asio::this_coro::executor;
    auto stream = co_await connect(ssl_ctx, host, service);

    co_await write_get_request(http_proto_ctx, stream, host, target);

    auto parser = http_proto::response_parser{ http_proto_ctx };
    parser.reset();
    parser.start();

    co_await http_io::async_read_header(stream, parser);

    while(is_redirect(parser.get()))
    {
        auto resp = parser.get();
        if(auto it = resp.find(http_proto::field::location); it != resp.end())
        {
            auto url     = urls::parse_uri(it->value);
            auto host    = url->host();
            auto service = url->has_port() ? url->port() : url->scheme();
            auto target  =
                !url->encoded_target().empty() ? url->encoded_target() : "/";

            // TODO: reuse the connection when possible
            co_await stream.async_shutdown(asio::as_tuple);
            stream   = co_await connect(ssl_ctx, host, service);
            co_await write_get_request(http_proto_ctx, stream, host, target);

            parser.reset();
            parser.start();

            co_await http_io::async_read_header(stream, parser);
        }
        else
        {
            throw std::runtime_error{ "Bad redirect response"};
        }
    }

    for(;;)
    {
        for(auto cb : parser.pull_body())
        {
            std::cout.write(static_cast<const char*>(
                cb.data()), cb.size());
            parser.consume_body(cb.size());
        }

        if(parser.is_complete())
            break;

        auto [ec, _] = co_await http_io::async_read_some(
            stream, parser, asio::as_tuple);
        if(ec && ec != http_proto::condition::need_more_input)
            throw boost::system::system_error{ ec };
    }

    auto [ec] = co_await stream.async_shutdown(asio::as_tuple);
    if(ec && ec != ssl::error::stream_truncated)
        throw boost::system::system_error{ ec };
};

int
main(int argc, char* argv[])
{
    try
    {
        if(argc != 2)
        {
            std::cerr << "Usage: flex_await <url>\n"
                      << "Example:\n"
                      << "    flex_await https://www.example.com\n";
            return EXIT_FAILURE;
        }

        auto url = urls::parse_uri(argv[1]);
        if(url.has_error())
        {
            std::cerr << "Failed to parse URL\n"
                      << "Error: " << url.error().what() << std::endl;
            return EXIT_FAILURE;
        }
        auto host    = url->host();
        auto service = url->has_port() ? url->port() : url->scheme();
        auto target =
            !url->encoded_target().empty() ? url->encoded_target() : "/";

        auto ioc            = asio::io_context{};
        auto ssl_ctx        = ssl::context{ ssl::context::tlsv12_client };
        auto http_proto_ctx = http_proto::context{};

        ssl_ctx.set_verify_mode(ssl::verify_none);

        {
            http_proto::response_parser::config cfg;
            cfg.body_limit = std::numeric_limits<std::size_t>::max();
            cfg.min_buffer = 1024 * 1024;
            #ifdef BOOST_HTTP_PROTO_HAS_ZLIB
            cfg.apply_gzip_decoder = true;
            cfg.apply_deflate_decoder = true;
            http_proto::zlib::install_service(http_proto_ctx);
            #endif
            http_proto::install_parser_service(http_proto_ctx, cfg);
        }

        asio::co_spawn(
            ioc,
            request(ssl_ctx, http_proto_ctx, host, service, target),
            [](std::exception_ptr ep)
            {
                if(ep)
                    std::rethrow_exception(ep);
            });

        ioc.run();
    }
    catch(std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

#else

int
main(int, char*[])
{
    std::cerr << "Coroutine examples require C++20" << std::endl;
    return EXIT_FAILURE;
}

#endif
