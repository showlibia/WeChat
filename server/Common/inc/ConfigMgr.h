//
// Created by matrix on 2/8/25.
//

#ifndef SERVER_CONFIGMGR_H
#define SERVER_CONFIGMGR_H

#include <filesystem>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <map>
#include <iostream>
#include "const.h"

struct SectionInfo {
    SectionInfo() = default;
    ~SectionInfo() {
        _section_datas.clear();
    }

    SectionInfo(const SectionInfo& other) {
        _section_datas = other._section_datas;
    }
    SectionInfo& operator=(const SectionInfo& other) {
        if(&other == this) {
            return *this;
        }
        _section_datas = other._section_datas;
        return *this;
    }

    std::map<std::string, std::string> _section_datas;
    std::string operator[](const std::string& key) {
        if(_section_datas.find(key) == _section_datas.end()) {
            return "";
        }
        return _section_datas[key];
    }
};

class ConfigMgr {
public:

    ~ConfigMgr() {
        _config_map.clear();
    }
    SectionInfo operator[](const std::string& section) {
        if(_config_map.find(section) == _config_map.end()) {
            return SectionInfo();
        }
        return _config_map[section];
    }
    ConfigMgr(const ConfigMgr& other) {
        _config_map = other._config_map;
    }
    ConfigMgr& operator=(const ConfigMgr& other) {
        if (&other == this) {
            return *this;
        }
        _config_map = other._config_map;
        return *this;
    }
    static ConfigMgr& Instance() {
        static ConfigMgr configMgr;
        return configMgr;
    }
private:
    ConfigMgr();
    // 存储section和key-value的映射
    std::map<std::string, SectionInfo> _config_map;
};


#endif //SERVER_CONFIGMGR_H
