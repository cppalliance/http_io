#
# Copyright (c) 2016-2017 Vinnie Falco (vinnie dot falco at gmail dot com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/boostorg/beast
#

source_group("" FILES
    http_client_sync.cpp
    )

add_executable (http-client-sync
    http_client_sync.cpp
)

set_property(
    TARGET http-client-sync
    PROPERTY FOLDER "example-client")

target_link_libraries(http-client-sync Boost::http_io)
