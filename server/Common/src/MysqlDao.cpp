#include "MysqlDao.h"
#include "ConfigMgr.h"
#include <iostream>
#include <mariadb/conncpp/PreparedStatement.hpp>
#include <mariadb/conncpp/ResultSet.hpp>
#include <mariadb/conncpp/Statement.hpp>
#include <memory>

MysqlDao::MysqlDao() {
  auto &cfg = ConfigMgr::Instance();
  const auto &host = cfg["Mysql"]["host"];
  const auto &port = cfg["Mysql"]["port"];
  const auto &user = cfg["Mysql"]["user"];
  const auto &password = cfg["Mysql"]["password"];
  const auto &schema = cfg["Mysql"]["schema"];
  _pool = std::make_unique<MysqlPool>(host + ":" + port, user, password, schema, 5);
}

int MysqlDao::Register(const std::string &name, const std::string &password,
                       const std::string &email) {
  auto conn = _pool->GetConnection();
  
  try {
    if (conn == nullptr) {
      return 0;
    }

    // 调用存储过程
    std::unique_ptr<sql::PreparedStatement> pstmt(
        conn->_connection->prepareStatement("CALL reg_user(?,?,?,@result)"));
    pstmt->setString(1, name);
    pstmt->setString(2, email);
    pstmt->setString(3, password);

    pstmt->execute();

    std::unique_ptr<sql::Statement> stmt(conn->_connection->createStatement());
    std::unique_ptr<sql::ResultSet> res(
        stmt->executeQuery("SELECT @result AS result"));
    if (res->next()) {
        int result = res->getInt("result");
        std::cout << "Result: " << result << std::endl;
        return result;
    }
    return -1;
  } catch (sql::SQLException &e) {
    std::cerr << "SQLException: " << e.what();
    std::cerr << " (MySQL error code: " << e.getErrorCode();
    std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    return -1;
  }
}

bool MysqlDao::CheckEmail(const std::string &email, const std::string &name) {
  auto conn = _pool->GetConnection();

  try {
    if (conn == nullptr) {
      return false;
    }

    std::unique_ptr<sql::PreparedStatement> pstmt(
        conn->_connection->prepareStatement(
            "SELECT email FROM user WHERE name = ?"));
    pstmt->setString(1, name);

    std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
    if (res->next()) {
      return res->getString("email") == email;
    }
  } catch (sql::SQLException &e) {
    std::cerr << "SQLException: " << e.what();
    std::cerr << " (MySQL error code: " << e.getErrorCode();
    std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    return false;
  }
}

bool MysqlDao::UpdatePwd(const std::string &name, const std::string &password) {
  auto conn = _pool->GetConnection();

  try {
    if (conn == nullptr) {
      return 0;
    }

    std::unique_ptr<sql::PreparedStatement> pstmt(
        conn->_connection->prepareStatement(
            "UPDATE user SET pwd = ? WHERE name = ?"));
    pstmt->setString(1, password);
    pstmt->setString(2, name);

    return pstmt->executeUpdate();

  } catch (sql::SQLException &e) {
    std::cerr << "SQLException: " << e.what();
    std::cerr << " (MySQL error code: " << e.getErrorCode();
    std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    return false;
  }
}

bool MysqlDao::CheckPwd(const std::string &email, const std::string &password, UserInfo &user_info) {
  auto conn = _pool->GetConnection();

  try {
    if (conn == nullptr) {
      return false;
    }

    std::unique_ptr<sql::PreparedStatement> pstmt(
        conn->_connection->prepareStatement(
            "SELECT name, pwd, email, uid FROM user WHERE email = ?"));
    pstmt->setString(1, email);

    std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
    if (res->next()) {
      if (res->getString("pwd") != password) {
        return false;
      }
      user_info.name = res->getString("name");
      user_info.password = res->getString("pwd");
      user_info.email = res->getString("email");
      user_info.uid = res->getInt("uid");
      return true;
    }
  } catch (sql::SQLException &e) {
    std::cerr << "SQLException: " << e.what();
    std::cerr << " (MySQL error code: " << e.getErrorCode();
    std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    return false;
  }
}