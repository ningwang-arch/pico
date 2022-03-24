cmake_minimum_required(VERSION 3.0)
project(pico)

include(cmake/utils.cmake)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_FLAGS
    "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations"
)
set(CMAKE_C_FLAGS
    "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations"
)

include_directories(.)

set(LIB_SRC
    pico/fiber.cc
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
    pico/thread.cc
    pico/util.cc)

add_library(pico SHARED ${LIB_SRC})

force_redefine_file_macro_for_sources(pico)

set(LIBS pico)

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