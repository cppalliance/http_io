#
# Copyright (c) 2023 Vinnie Falco (vinnie dot falco at gmail dot com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/CPPAlliance/http_io
#

source_group("" FILES
    server.cpp
    )

add_executable (server
    server.cpp
)
target_compile_definitions(
    server PRIVATE BOOST_ASIO_NO_DEPRECATED)

set_property(
    TARGET server
    PROPERTY FOLDER "examples")

target_link_libraries(server Boost::http_io)
