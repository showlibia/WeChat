cmake_minimum_required(VERSION 3.28)
project(ChatServer LANGUAGES CXX)

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

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../Common ${CMAKE_CURRENT_BINARY_DIR}/CommonLib)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/self.ini ${CMAKE_BINARY_DIR}/self.ini COPYONLY)

set(HEADERS
    inc/CServer.h
    inc/LogicSystem.h
    inc/CSession.h
    inc/MsgNode.h
    inc/ChatGrpcClient.h
    inc/ChatServiceImpl.h
    inc/UserMgr.h
)

set(SOURCES
    src/CServer.cpp
    src/LogicSystem.cpp
    src/ChatServer.cpp
    src/CSession.cpp
    src/MsgNode.cpp
    src/ChatGrpcClient.cpp
    src/ChatServiceImpl.cpp
    src/UserMgr.cpp
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../Common/proto
    ${CMAKE_CURRENT_SOURCE_DIR}/../Common/inc
)

add_executable(ChatServer
    ${SOURCES}
    ${HEADERS}
)

target_link_libraries(ChatServer PRIVATE CommonLib)

target_include_directories(ChatServer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../Common/proto
    ${CMAKE_CURRENT_SOURCE_DIR}/../Common/inc
)