/**
 * @file StatusServer.cpp
 * @brief 用来监听其他服务器的查询请求的状态服务
 */

#include "ConfigMgr.h"
#include "StatusServiceImpl.h"
#include "Logger.h"

void RunServer() {
  auto &cfg = ConfigMgr::Instance();
  std::string server_address(cfg["StatusServer"]["host"] + ":" +
                             cfg["StatusServer"]["port"]);
  StatusServiceImpl service;

  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);

  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  LOG(info) << "Server listening on " << server_address << std::endl;

  boost::asio::io_context io_context;
  boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);

  signals.async_wait(
      [&server](const boost::system::error_code &error, int signal_number) {
        if (!error) {
          LOG(info) << "Shutting down server..." << std::endl;
          server->Shutdown(); // 优雅地关闭服务器
        }
      });

  std::thread([&io_context]() { io_context.run(); }).detach();
  // 等待服务器关闭
  server->Wait();
  io_context.stop(); // 停止io_context
}

int main(int argc, char **argv) {
  Logger::Init("../../StatusServer.log");
  try {
    RunServer();
  } catch (std::exception const &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return 0;
}