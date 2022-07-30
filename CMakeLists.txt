cmake_minimum_required(VERSION 3.0)
project(pico)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(cmake/utils.cmake)
include(cmake/FindMySQL.cmake)

include_directories(.)

set(SOURCE_FILES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/pico)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS
  "$ENV{CXXFLAGS} -rdynamic -O2 -fPIC -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations"
)
set(CMAKE_C_FLAGS
  "$ENV{CXXFLAGS} -rdynamic -O2 -fPIC -ggdb -std=c11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations"
)

option(BUILD_TEST "ON for complile test" OFF)

# find dependencies
find_package(MySQL REQUIRED)

if(MYSQL_FOUND)
  include_directories(${MYSQL_INCLUDE_DIR})
endif()

find_package(OpenSSL REQUIRED)

if(OPENSSL_FOUND)
  include_directories(${OPENSSL_INCLUDE_DIR})
  link_directories(${OPENSSL_LIBRARY_DIR})
endif()

find_package(ZLIB REQUIRED)

if(ZLIB_FOUND)
  include_directories(${ZLIB_INCLUDE_DIR})
endif()

find_package(yaml-cpp REQUIRED)
include_directories(${YAML_INCLUDE_DIR})
link_directories(${YAML_LIBRARY_DIR})

find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)
include_directories(${JSONCPP_INCLUDE_DIR})

# find source files
include_sub_directories_recursively(${SOURCE_FILES_DIR})

file(GLOB_RECURSE LIB_SRC ${SOURCE_FILES_DIR}/*.cpp ${SOURCE_FILES_DIR}/*.cc ${SOURCE_FILES_DIR}/*.hpp ${SOURCE_FILES_DIR}/*.h)

add_library(pico SHARED ${LIB_SRC})
force_redefine_file_macro_for_sources(pico)

set(LIBS
  pico
  dl
  pthread
  ${ZLIB_LIBRARIES}
  ${JSONCPP_LIBRARIES}
  ${YAML_CPP_LIBRARIES}
  ${OPENSSL_LIBRARIES}
  ${MYSQL_LIBRARIES}
  sqlite3
  crypto
)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

if(BUILD_TEST)
  build_test_target(test_log "tests/test_log.cc" pico "${LIBS}")
  build_test_target(test_thread "tests/test_thread.cc" pico "${LIBS}")
  build_test_target(test_fiber "tests/test_fiber.cc" pico "${LIBS}")
  build_test_target(test_scheduler "tests/test_scheduler.cc" pico "${LIBS}")
  build_test_target(test_iomanager "tests/test_iomanager.cc" pico "${LIBS}")
  build_test_target(test_hook "tests/test_hook.cc" pico "${LIBS}")
  build_test_target(test_address "tests/test_address.cc" pico "${LIBS}")
  build_test_target(test_socket "tests/test_socket.cc" pico "${LIBS}")
  build_test_target(test_tcp_server "tests/test_tcp_server.cc" pico "${LIBS}")
  build_test_target(test_http "tests/test_http.cc" pico "${LIBS}")
  build_test_target(test_http_parser "tests/test_http_parser.cc" pico "${LIBS}")
  build_test_target(test_http_connection "tests/test_http_connection.cc" pico "${LIBS}")
  build_test_target(test_http_server "tests/test_http_server.cc" pico "${LIBS}")
  build_test_target(test_config "tests/test_config.cc" pico "${LIBS}")
  build_test_target(test_register "tests/test_register.cc" pico "${LIBS}")
  build_test_target(test_jwt "tests/test_jwt.cc" pico "${LIBS}")
  build_test_target(test_date "tests/test_date.cc" pico "${LIBS}")
  build_test_target(test_mustache "tests/test_mustache.cc" pico "${LIBS}")
  build_test_target(test_mapper "tests/test_mapper.cc" pico "${LIBS}")
  build_test_target(test_ws_server "tests/test_ws_server.cc" pico "${LIBS}")
  build_test_target(test_ws_client "tests/test_ws_client.cc" pico "${LIBS}")
  build_test_target(test_sqlite3 "tests/test_sqlite3.cc" pico "${LIBS}")
  build_test_target(test_fuzzy_match "tests/test_fuzzy_match.cc" pico "${LIBS}")
endif()

add_executable(main pico/main.cc)
add_dependencies(main pico)
force_redefine_file_macro_for_sources(main)
target_link_libraries(main ${LIBS})
