#
# Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/vinniefalco/http_io
#

set(EXTRAFILES
    ../../url/extra/test_main.cpp
    ../../url/extra/test_suite.hpp
    )

set(PFILES
    CMakeLists.txt
    #Jamfile
    buffer.cpp
    error.cpp
    read.cpp
    sandbox.cpp
    write.cpp
    )

find_package(OpenSSL REQUIRED)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "" FILES ${PFILES})
source_group("_extra" FILES ${EXTRAFILES})
add_executable(boost_http_io_tests ${PFILES} ${EXTRAFILES})
target_include_directories(boost_http_io_tests PRIVATE . ../../url/extra)
target_compile_definitions(boost_http_io_tests PRIVATE BOOST_ASIO_NO_DEPRECATED)
target_link_libraries(boost_http_io_tests PRIVATE
    OpenSSL::SSL
    Boost::http_io)
