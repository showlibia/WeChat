//
// Created by matrix on 2/7/25.
//

#include "CServer.h"
#include "ConfigMgr.h"
#include "RedisMgr.h"

void TestRedisMgr() {
    assert(RedisMgr::GetInstance()->Auth("123456"));
    assert(RedisMgr::GetInstance()->Set("blogwebsite","llfc.club"));
    std::string value="";
    assert(RedisMgr::GetInstance()->Get("blogwebsite", value) );
    assert(RedisMgr::GetInstance()->Get("nonekey", value) == false);
    assert(RedisMgr::GetInstance()->HSet("bloginfo","blogwebsite", "llfc.club"));
    assert(RedisMgr::GetInstance()->HGet("bloginfo","blogwebsite") != "");
    assert(RedisMgr::GetInstance()->ExistsKey("bloginfo"));
    assert(RedisMgr::GetInstance()->Del("bloginfo"));
    assert(RedisMgr::GetInstance()->Del("bloginfo"));
    assert(RedisMgr::GetInstance()->ExistsKey("bloginfo") == false);
    assert(RedisMgr::GetInstance()->LPush("lpushkey1", "lpushvalue1"));
    assert(RedisMgr::GetInstance()->LPush("lpushkey1", "lpushvalue2"));
    assert(RedisMgr::GetInstance()->LPush("lpushkey1", "lpushvalue3"));
    assert(RedisMgr::GetInstance()->RPop("lpushkey1", value));
    assert(RedisMgr::GetInstance()->RPop("lpushkey1", value));
    assert(RedisMgr::GetInstance()->LPop("lpushkey1", value));
    assert(RedisMgr::GetInstance()->LPop("lpushkey2", value)==false);
}

int main() {
    TestRedisMgr();
    ConfigMgr& gCfgMgr = ConfigMgr::Instance();
    std::string gate_port_str = gCfgMgr["GateServer"]["port"];
    unsigned short gate_port = std::stoi(gate_port_str);
    try {
        unsigned short port = static_cast<unsigned short>(gate_port);
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