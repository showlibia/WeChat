//
// Created by matrix on 2/7/25.
//

#include "CServer.h"
#include "const.h"

int main() {
    try {
        unsigned short port = static_cast<unsigned short>(12345);
        net::io_context ioc{1};
        net::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc](const boost::system::error_code& error, int signal_number){
            if(error) {
                return ;
            }
            ioc.stop();
        });
        std::make_shared<CServer>(ioc, port)->Start();
        std::cout << "GateServer start on port: " << port << std::endl;
        ioc.run();
    } catch (std::exception & e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}