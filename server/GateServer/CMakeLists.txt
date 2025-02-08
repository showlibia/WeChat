cmake_minimum_required(VERSION 3.28)
project(server)

set(CMAKE_CXX_STANDARD 17)

set(JSONCPP_INCLUDE_DIRS /usr/include/json)
set(JSONCPP_LIBRARIES /usr/lib/libjsoncpp.so)

set(GENERATED_SRC message.pb.cc message.grpc.pb.cc)
set(GENERATED_HDR message.pb.h message.grpc.pb.h
        ConfigMgr.h
        ConfigMgr.cpp)

find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

configure_file(config.ini ${CMAKE_BINARY_DIR}/config.ini COPYONLY)

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
        LogicSystem.cpp
        VerifyGrpcClient.h
        VerifyGrpcClient.cpp
        ${GENERATED_SRC}
        ${GENERATED_HDR})

target_link_libraries(server ${JSONCPP_LIBRARIES} gRPC::grpc++ protobuf::libprotobuf)