//
// Created by matrix on 2/8/25.
//

#include "ConfigMgr.h"

void ConfigMgr::LoadConfig(const std::filesystem::path &config_path) {
  boost::property_tree::ptree pt;
  try {
    boost::property_tree::read_ini(config_path.string(), pt);
  } catch (const boost::property_tree::ini_parser_error &e) {
    throw std::runtime_error("Failed to load config file: " +
                             std::string(e.what()));
  }

  for (const auto &section_pair : pt) {
    const std::string &section_name = section_pair.first;
    const boost::property_tree::ptree &section_tree = section_pair.second;

    SectionInfo &sectionInfo = _config_map[section_name]; 
    for (const auto &key_value : section_tree) {
      const std::string &key = key_value.first;
      const std::string &value = key_value.second.data();
      sectionInfo._section_datas[key] = value;
    }
  }
}