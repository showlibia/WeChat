//
// Created by matrix on 2/8/25.
//

#include "ConfigMgr.h"

ConfigMgr::ConfigMgr() {
    std::filesystem::path current_path = std::filesystem::current_path();
    std::filesystem::path config_path = current_path / "config.ini";
//    std::cout << "Config path: " << config_path << std::endl;

    boost::property_tree::ptree pt;
    boost::property_tree::read_ini(config_path.string(), pt);

    for(const auto& section_pair: pt) {
        const std::string & section_name = section_pair.first;
        const boost::property_tree::ptree & section_tree = section_pair.second;

        SectionInfo sectionInfo;
        for(const auto& key_value: section_tree) {
            const std::string & key = key_value.first;
            const std::string & value = key_value.second.data();
            sectionInfo._section_datas[key] = value;
        }

        _config_map[section_name] = sectionInfo;
    }

    for(const auto& section_entry: _config_map) {
        const std::string & section_name = section_entry.first;
        const SectionInfo & section_info = section_entry.second;
        std::cout << "[" << section_name << "]" << std::endl;
        for(const auto& data_entry: section_info._section_datas) {
            const std::string & key = data_entry.first;
            const std::string & value = data_entry.second;
            std::cout << key << "=" << value << std::endl;
        }
    }
}