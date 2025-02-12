cmake_minimum_required(VERSION 3.28)
project(server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "未指定构建类型，默认使用 Debug 模式")
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "构建类型 (Debug, Release)" FORCE)
endif()
message(STATUS "当前构建类型: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

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
    ${CMAKE_CURRENT_SOURCE_DIR}/message.pb.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/message.grpc.pb.cc
)
set(GENERATED_HDR
    ${CMAKE_CURRENT_SOURCE_DIR}/message.pb.h
    ${CMAKE_CURRENT_SOURCE_DIR}/message.grpc.pb.h
)
foreach(file ${GENERATED_SRC} ${GENERATED_HDR})
    set_source_files_properties(${file} PROPERTIES GENERATED TRUE)
endforeach()

set(SOURCES
    CServer.cpp
    HttpConnection.cpp
    GateServer.cpp
    LogicSystem.cpp
    VerifyGrpcClient.cpp
    ConfigMgr.cpp
    AsioIOContextPool.cpp
    RedisMgr.cpp
    RedisConPool.cpp
    RPCConPool.cpp
    MysqlPool.cpp
    MysqlDao.cpp
    MysqlMgr.cpp
    ${GENERATED_SRC}
)
set(HEADERS
    CServer.h
    const.h
    HttpConnection.h
    Singleton.h
    LogicSystem.h
    VerifyGrpcClient.h
    ConfigMgr.h
    AsioIOContextPool.h
    RedisMgr.h
    RedisConPool.h
    RPCConPool.h
    MysqlPool.h
    MysqlDao.h
    MysqlMgr.h
    ${GENERATED_HDR}
)

add_executable(server
    ${SOURCES}
    ${HEADERS}
)

target_include_directories(server PRIVATE
    ${JSONCPP_INCLUDE_DIRS}     
    ${HIREDIS_INCLUDE_DIRS}    
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(server PRIVATE
    ${JSONCPP_LIBRARIES}
    gRPC::grpc++
    protobuf::libprotobuf
    ${HIREDIS_LIBRARIES}
    ${MARIADBCPP_LIBRARIES}
)