cmake_minimum_required(VERSION 3.6)
project(gjserver)


set(CMAKE_CXX_STANDARD 20)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

if(LINUX)
    set(CMAKE_CXX_COMPILER clang++)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe -Wall -Wextra")
endif()

if(WIN32)
    # pls god, kill it
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()


add_compile_options("-pthread")

set(HEADER_FILES
    ./src/gjserver/client/client.h
    ./src/gjserver/client/clientconfig.h
    ./src/gjserver/client/gjclient.h
    ./src/gjserver/concurrency/service.h
    ./src/gjserver/concurrency/threadpool.h
    ./src/gjserver/kvstorage/kvstorage.h
    ./src/gjserver/kvstorage/kvstoragefile.h
    ./src/gjserver/logging/log.h
    ./src/gjserver/logging/stat.h
    ./src/gjserver/net/proto.h
    ./src/gjserver/net/session.h
    ./src/gjserver/server/gjserver.h
    ./src/gjserver/server/server.h
    ./src/gjserver/utils/time.h
)

set(SOURCE_FILES
    ./src/gjserver/client/client.cpp
    ./src/gjserver/client/clientconfig.cpp
    ./src/gjserver/client/gjclient.cpp
    ./src/gjserver/concurrency/service.cpp
    ./src/gjserver/concurrency/threadpool.cpp
    ./src/gjserver/kvstorage/kvstorage.cpp
    ./src/gjserver/kvstorage/kvstoragefile.cpp
    ./src/gjserver/logging/log.cpp
    ./src/gjserver/logging/stat.cpp
    ./src/gjserver/net/proto.cpp
    ./src/gjserver/server/gjserver.cpp
    ./src/gjserver/server/server.cpp
    ./src/gjserver/utils/time.cpp
)

set(TEST_SOURCE_FILES
    ./src/gjserver/ut/main.cpp
    ./src/gjserver/ut/tests.cpp
)

add_library(asio INTERFACE)
target_include_directories(asio INTERFACE contrib/asio/asio/include)
#target_compile_definitions(asio INTERFACE ASIO_STANDALONE ASIO_NO_DEPRECATED)
target_compile_definitions(asio INTERFACE ASIO_STANDALONE)

add_executable(gjserveruttest ${HEADER_FILES} ${SOURCE_FILES} ${TEST_SOURCE_FILES})
target_include_directories(gjserveruttest PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src)
target_link_libraries(gjserveruttest PRIVATE gtest gtest_main asio)

add_executable(gjserver ${HEADER_FILES} ${SOURCE_FILES} ./src/gjserver/apps/mainserver.cpp )
target_include_directories(gjserver PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src)
target_link_libraries(gjserver PRIVATE asio)

add_executable(gjclient ${HEADER_FILES} ${SOURCE_FILES} ./src/gjserver/apps/mainclient.cpp )
target_include_directories(gjclient PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src)
target_link_libraries(gjclient PRIVATE asio)

if(WIN32)
    include(GoogleTest)
    gtest_discover_tests(gjserveruttest)
endif()
