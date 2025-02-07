cmake_minimum_required(VERSION 3.28)
project(server)

set(CMAKE_CXX_STANDARD 17)

set(JSONCPP_INCLUDE_DIRS /usr/include/json)
set(JSONCPP_LIBRARIES /usr/lib/libjsoncpp.so)

include_directories(${JSONCPP_INCLUDE_DIRS})

add_executable(server
        CServer.h
        CServer.cpp
        const.h
        HttpConnection.h
        HttpConnection.cpp
        GateServer.cpp
        Singleton.h
        LogicSystem.h
        LogicSystem.cpp)

target_link_libraries(server ${JSONCPP_LIBRARIES})