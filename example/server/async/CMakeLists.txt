#
# Copyright (c) 2022 Vinnie Falco (vinnie dot falco at gmail dot com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/CPPAlliance/http_io
#

source_group("" FILES
    http_server_async.cpp
    )

add_executable (http-server-async
    http_server_async.cpp
)
target_compile_definitions(
    http-server-async PRIVATE BOOST_ASIO_NO_DEPRECATED)

set_property(
    TARGET http-server-async
    PROPERTY FOLDER "example-server")

target_link_libraries(http-server-async Boost::http_io)
