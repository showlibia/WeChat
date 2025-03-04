#ifndef LOGGER_H
#define LOGGER_H

#include <boost/log/attributes.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <filesystem>
#include <string>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace keywords = boost::log::keywords;
namespace expr = boost::log::expressions;

class Logger {
public:
  // 初始化日志系统
  static void Init(const std::string &log_file = "") {
    // 添加时间戳属性
    logging::add_common_attributes();

    // 设置日志格式
    auto format =
        (expr::stream << "["
                      << expr::format_date_time<boost::posix_time::ptime>(
                             "TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
                      << "] " << "[" << logging::trivial::severity << "] "
                      << "[" << expr::attr<std::string>("File") << ":"
                      << expr::attr<int>("Line") << "] " << "["
                      << expr::attr<std::string>("Function") << "] "
                      << expr::smessage);

    if (!log_file.empty()) {
      std::filesystem::path path(log_file);
      if (!std::filesystem::exists(path.parent_path())) {
        std::cerr << "[ERROR] ["<< __FILE__ << " : " << __FUNCTION__ << " : " << __LINE__ << " : " << "Log directory does not exist: " << path.parent_path()
                  << std::endl;
        std::filesystem::create_directories(path.parent_path());
        std::cout << "[INFO] [" << __FILE__ << " : " << __FUNCTION__ << " : "
                << __LINE__ << " : " << "Created log directory: " << path.parent_path()
                  << std::endl;
      }
      // 输出到文件
      std::filesystem::path abs_path = std::filesystem::absolute(path);

      std::cout << "[INFO] [" << __FILE__ << ":" << __FUNCTION__ << ":"
                << __LINE__ << "]" << "Logging to file: " << abs_path.string() << std::endl;
      logging::add_file_log(keywords::file_name = log_file,
                            keywords::open_mode = std::ios::app,
                            keywords::rotation_size = 10 * 1024 * 1024, // 10MB
                            keywords::format = format);
    } else {
      // 输出到控制台
      std::cout << "[INFO] [" << __FILE__ << " : " << __FUNCTION__ << " : "
                << __LINE__ << " : " << "Log to console" << std::endl;
      logging::add_console_log(std::cout, keywords::format = format);
    }
  }

  // 获取日志记录器
  static src::severity_logger<logging::trivial::severity_level> &GetLogger() {
    static src::severity_logger<logging::trivial::severity_level> logger;
    return logger;
  }
};

// 日志宏，方便使用
#define LOG(severity)                                                          \
  BOOST_LOG_SEV(Logger::GetLogger(), logging::trivial::severity)               \
      << boost::log::add_value("File", __FILE__)                               \
      << boost::log::add_value("Function", __FUNCTION__)                       \
      << boost::log::add_value("Line", __LINE__)

#endif // LOGGER_H
