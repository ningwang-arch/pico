cmake_minimum_required(VERSION 3.0)
project(pico)

include(cmake/utils.cmake)

include_directories(.)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_FLAGS
    "$ENV{CXXFLAGS} -rdynamic -O2 -fPIC -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations"
)
set(CMAKE_C_FLAGS
    "$ENV{CXXFLAGS} -rdynamic -O2 -fPIC -ggdb -std=c11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations"
)


set(LIB_SRC
    pico/address.cc
    pico/fdmanager.cc
    pico/fiber.cc
    pico/hook.cc
    pico/http/http_connection.cc
    pico/http/http_parser.cc
    pico/http/http.cc
    pico/http/http11_parser.cc
    pico/http/httpclient_parser.cc
    pico/iomanager.cc
    pico/log/LogAppender.cc
    pico/log/LogEvent.cc
    pico/log/Logger.cc
    pico/log/LoggerManager.cc
    pico/log/LogLevel.cc
    pico/log/PatternConverter.cc
    pico/log/PatternLayout.cc
    pico/log/SimpleLayout.cc
    pico/log/TTCCLayout.cc
    pico/mutex.cc
    pico/scheduler.cc
    pico/socket_stream.cc
    pico/socket.cc
    pico/tcp_server.cc
    pico/thread.cc
    pico/timer.cc
    pico/uri.cc
    pico/util.cc)

add_library(pico SHARED ${LIB_SRC})

force_redefine_file_macro_for_sources(pico)

set(LIBS
    pico
    dl
    )

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

add_executable(test_log tests/test_log.cc)
add_dependencies(test_log pico)
force_redefine_file_macro_for_sources(test_log)
target_link_libraries(test_log ${LIBS})

add_executable(test_thread tests/test_thread.cc)
add_dependencies(test_thread pico)
force_redefine_file_macro_for_sources(test_thread)
target_link_libraries(test_thread ${LIBS})

add_executable(test_fiber tests/test_fiber.cc)
add_dependencies(test_fiber pico)
force_redefine_file_macro_for_sources(test_fiber)
target_link_libraries(test_fiber ${LIBS})

add_executable(test_scheduler tests/test_scheduler.cc)
add_dependencies(test_scheduler pico)
force_redefine_file_macro_for_sources(test_scheduler)
target_link_libraries(test_scheduler ${LIBS})

add_executable(test_iomanager tests/test_iomanager.cc)
add_dependencies(test_iomanager pico)
force_redefine_file_macro_for_sources(test_iomanager)
target_link_libraries(test_iomanager ${LIBS})

add_executable(test_hook tests/test_hook.cc)
add_dependencies(test_hook pico)
force_redefine_file_macro_for_sources(test_hook)
target_link_libraries(test_hook ${LIBS})

add_executable(test_address tests/test_address.cc)
add_dependencies(test_address pico)
force_redefine_file_macro_for_sources(test_address)
target_link_libraries(test_address ${LIBS})

add_executable(test_socket tests/test_socket.cc)
add_dependencies(test_socket pico)
force_redefine_file_macro_for_sources(test_socket)
target_link_libraries(test_socket ${LIBS})

add_executable(test_tcp_server tests/test_tcp_server.cc)
add_dependencies(test_tcp_server pico)
force_redefine_file_macro_for_sources(test_tcp_server)
target_link_libraries(test_tcp_server ${LIBS})

add_executable(test_http tests/test_http.cc)
add_dependencies(test_http pico)
force_redefine_file_macro_for_sources(test_http)
target_link_libraries(test_http ${LIBS})

add_executable(test_http_parser tests/test_http_parser.cc)
add_dependencies(test_http_parser pico)
force_redefine_file_macro_for_sources(test_http_parser)
target_link_libraries(test_http_parser ${LIBS})

# /home/eclipse/code/vscode/pico/tests/test_http_connection.cc
add_executable(test_http_connection tests/test_http_connection.cc)
add_dependencies(test_http_connection pico)
force_redefine_file_macro_for_sources(test_http_connection)
target_link_libraries(test_http_connection ${LIBS})