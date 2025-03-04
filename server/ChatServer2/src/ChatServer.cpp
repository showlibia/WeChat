#include "AsioIOContextPool.h"
#include "CServer.h"
#include "ChatServiceImpl.h"
#include "ConfigMgr.h"
#include "Logger.h"
#include "RedisMgr.h"
#include "const.h"
#include <boost/log/trivial.hpp>
#include <csignal>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server_builder.h>
#include <iostream>
#include <memory>
#include <mutex>

bool bstop = false;
std::condition_variable cond_quit;
std::mutex mutex_quit;

int main() {
  Logger::Init("../../ChatServer2.log"); // 初始化日志系统
  try {
    auto &cfg = ConfigMgr::Instance();
    cfg.LoadConfig("self.ini");
    auto server_name = cfg["SelfServer"]["name"];
    auto pool = AsioIOContextPool::GetInstance();
    RedisMgr::GetInstance()->HSet(LOGIN_COUNT, server_name, "0");

    std::string server_address(cfg["SelfServer"]["host"] + ":" +
                               cfg["SelfServer"]["rpcport"]);
    ChatServiceImpl service;
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    // 构建并启动gRPC服务器
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    LOG(info) << "RPC Server listening on " << server_address << std::endl;

    // 单独启动一个线程处理grpc服务
    std::thread grpc_server_thread([&server] { server->Wait(); });

    boost::asio::io_context io_context;
    boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
    signals.async_wait([&io_context, pool, &server](auto, auto) {
      io_context.stop();
      pool->Stop();
      server->Shutdown();
    });
    auto port_str = cfg["SelfServer"]["port"];
    CServer s(io_context, atoi(port_str.c_str()));
    io_context.run();

    RedisMgr::GetInstance()->HDel(LOGIN_COUNT, server_name);
    grpc_server_thread.join();
  } catch (std::exception &e) {
    LOG(warning) << "Exception: " << e.what() << std::endl;
  }
}