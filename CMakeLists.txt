#
# Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/vinniefalco/http_io
#

cmake_minimum_required(VERSION 3.5...3.16)

set(BOOST_HTTP_IO_VERSION 1)
if(BOOST_SUPERPROJECT_VERSION)
    set(BOOST_HTTP_IO_VERSION ${BOOST_SUPERPROJECT_VERSION})
endif()

project(boost_http_io VERSION "${BOOST_HTTP_IO_VERSION}" LANGUAGES CXX)

file(GLOB_RECURSE BOOST_HTTP_IO_HEADERS $<$<VERSION_GREATER_EQUAL:${CMAKE_VERSION},3.12>:CONFIGURE_DEPENDS>
    include/boost/*.hpp
    include/boost/*.ipp
    include/boost/*.natvis
)

set(BOOST_HTTP_IO_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/src.cpp
)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/include/boost PREFIX "" FILES ${BOOST_HTTP_IO_HEADERS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX "" FILES ${BOOST_HTTP_IO_SOURCES})

# TODO: For Boost superproject, do we want to support header-only mode?
#       Then, this needs to read `add_library(boost_http_io INTERFACE)`
#       and related settings need to be INTERFACE-ed as well.
add_library(boost_http_io ${BOOST_HTTP_IO_HEADERS} ${BOOST_HTTP_IO_SOURCES})
add_library(Boost::http_io ALIAS boost_http_io)

target_compile_features(boost_http_io PUBLIC cxx_constexpr)

# TODO: For Boost superproject, this may need to be INTERFACE setting.
include(GNUInstallDirs)
if(BOOST_SUPERPROJECT_VERSION)
    target_include_directories(boost_http_io PUBLIC include)
else()
    target_include_directories(boost_http_io
        PUBLIC
            "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    )
endif()

target_compile_definitions(boost_http_io PUBLIC BOOST_HTTP_IO_NO_LIB=1)

if(BUILD_SHARED_LIBS)
    target_compile_definitions(boost_http_io PUBLIC BOOST_HTTP_IO_DYN_LINK=1)
else()
    target_compile_definitions(boost_http_io PUBLIC BOOST_HTTP_IO_STATIC_LINK=1)
endif()

if(BOOST_SUPERPROJECT_VERSION)
    #
    # Building as part of Boost superproject tree, with Boost as dependency.
    #
    target_link_libraries(boost_http_io
        PUBLIC
            Boost::align
            Boost::assert
            Boost::config
            Boost::container
            Boost::exception
            Boost::system
            Boost::throw_exception
            Boost::utility
    )

elseif(BOOST_HTTP_IO_IN_BOOST_TREE)
    #
    # Building inside Boost tree, out of Boost superproject tree, with Boost as dependency.
    # e.g. on Travis or other CI, or when producing Visual Studio Solution and Projects.
    #
    get_filename_component(BOOST_ROOT ../.. ABSOLUTE)
    target_include_directories(boost_http_io PUBLIC ${BOOST_ROOT})
    target_link_directories(boost_http_io PUBLIC ${BOOST_ROOT}/stage/lib)

else()
    #
    # Building out of Boost tree, out of Boost superproject tree, with Boost as dependency.
    # e.g. for packaging or added with add_subdirectory.
    #
    find_package(Boost REQUIRED COMPONENTS container system)
    target_link_libraries(boost_http_io
        PUBLIC
            Boost::container
            Boost::system
    )
endif()

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR AND NOT BOOST_HTTP_IO_IN_BOOST_TREE)
    set_target_properties(boost_http_io PROPERTIES EXPORT_NAME http_io)
    install(TARGETS boost_http_io
        EXPORT boost_http_io_targets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

    install(EXPORT boost_http_io_targets
        FILE boost_http_io-targets.cmake
        NAMESPACE Boost::
        DESTINATION lib/cmake/boost_http_io
    )

    include(CMakePackageConfigHelpers)

    configure_package_config_file(cmake/config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/boost_http_io-config.cmake
        INSTALL_DESTINATION lib/cmake/boost_http_io
    )

    write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/boost_http_io-config-version.cmake
        VERSION ${PROJECT_VERSION} COMPATIBILITY SameMajorVersion
    )

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/boost_http_io-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/boost_http_io-config-version.cmake
        DESTINATION lib/cmake/boost_http_io
    )

    install(DIRECTORY include/ DESTINATION include)
endif()

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(test)
endif()

#add_subdirectory (example)
