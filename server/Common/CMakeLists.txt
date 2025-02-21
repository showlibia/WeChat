cmake_minimum_required(VERSION 3.28)
project(CommonLib LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "未指定构建类型，默认使用 Debug 模式")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "构建类型 (Debug, Release)" FORCE)
endif()

message(STATUS "当前构建类型: ${CMAKE_BUILD_TYPE}")

set(JSONCPP_INCLUDE_DIRS /usr/include/json)
set(JSONCPP_LIBRARIES /usr/lib/libjsoncpp.so)

set(MARIADBCPP_INCLUDE_DIRS /usr/include/mariadb)
set(MARIADBCPP_LIBRARIES /usr/lib/libmariadbcpp.so)

find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(HIREDIS REQUIRED hiredis)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.ini ${CMAKE_BINARY_DIR}/config.ini COPYONLY)

set(GENERATED_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/proto/message.pb.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/proto/message.grpc.pb.cc
)
set(GENERATED_HDR
    ${CMAKE_CURRENT_SOURCE_DIR}/proto/message.pb.h
    ${CMAKE_CURRENT_SOURCE_DIR}/proto/message.grpc.pb.h
)

foreach(file ${GENERATED_SRC} ${GENERATED_HDR})
    set_source_files_properties(${file} PROPERTIES GENERATED TRUE)
endforeach()

set(HEADERS
    inc/AsioIOContextPool.h
    inc/ConfigMgr.h
    inc/const.h
    inc/MysqlDao.h
    inc/MysqlMgr.h
    inc/MysqlPool.h
    inc/RedisConPool.h
    inc/RedisMgr.h
    inc/RPCConPool.h
    inc/Singleton.h
    inc/StatusGrpcClient.h
    ${GENERATED_HDR}
)

set(SOURCES
    src/AsioIOContextPool.cpp
    src/ConfigMgr.cpp
    src/MysqlDao.cpp
    src/MysqlMgr.cpp
    src/MysqlPool.cpp
    src/RedisConPool.cpp
    src/RedisMgr.cpp
    src/StatusGrpcClient.cpp
    ${GENERATED_SRC}
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/proto
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

add_library(CommonLib STATIC ${SOURCES}
    ${HEADERS})

target_include_directories(CommonLib PUBLIC
    ${JSONCPP_INCLUDE_DIRS}
    ${HIREDIS_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/proto
)

target_link_libraries(CommonLib PUBLIC
    ${JSONCPP_LIBRARIES}
    gRPC::grpc++
    protobuf::libprotobuf
    ${HIREDIS_LIBRARIES}
    ${MARIADBCPP_LIBRARIES}
)