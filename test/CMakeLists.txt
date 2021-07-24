#
# Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/vinniefalco/http_io
#


set(TFILES
    ./include/main.cpp
    ./include/test_suite.hpp
    )

set(PFILES
    CMakeLists.txt
    error.cpp
    )

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "" FILES ${PFILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/include PREFIX "include" FILES ${TFILES})
add_executable(http_io_tests ${PFILES} ${TFILES})
target_include_directories(http_io_tests PRIVATE . ./include)
target_link_libraries(http_io_tests PRIVATE Boost::http_io)
