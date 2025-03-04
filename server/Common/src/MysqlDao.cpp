#include "MysqlDao.h"
#include "ConfigMgr.h"
#include "Logger.h"
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
  _pool =
      std::make_unique<MysqlPool>(host + ":" + port, user, password, schema, 5);
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
      LOG(info) << "Result: " << result << std::endl;
      return result;
    }
    return -1;
  } catch (sql::SQLException &e) {
    LOG(warning) << "SQLException: " << e.what();
    LOG(warning) << " (MySQL error code: " << e.getErrorCode();
    LOG(warning) << ", SQLState: " << e.getSQLState() << " )" << std::endl;
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
    LOG(warning) << "SQLException: " << e.what();
    LOG(warning) << " (MySQL error code: " << e.getErrorCode();
    LOG(warning) << ", SQLState: " << e.getSQLState() << " )" << std::endl;
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
    LOG(warning) << "SQLException: " << e.what();
    LOG(warning) << " (MySQL error code: " << e.getErrorCode();
    LOG(warning) << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    return false;
  }
}

bool MysqlDao::CheckPwd(const std::string &email, const std::string &password,
                        UserInfo &user_info) {
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
      user_info.pwd = res->getString("pwd");
      user_info.email = res->getString("email");
      user_info.uid = res->getInt("uid");
      return true;
    }
  } catch (sql::SQLException &e) {
    LOG(warning) << "SQLException: " << e.what();
    LOG(warning) << " (MySQL error code: " << e.getErrorCode();
    LOG(warning) << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    return false;
  }
}

std::shared_ptr<UserInfo> MysqlDao::GetUser(int uid) {
  auto conn = _pool->GetConnection();

  try {
    if (conn == nullptr) {
      return nullptr;
    }

    std::unique_ptr<sql::PreparedStatement> pstmt(
        conn->_connection->prepareStatement(
            "SELECT name, pwd, email, uid FROM user WHERE uid = ?"));
    pstmt->setInt(1, uid);

    std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
    if (res->next()) {
      auto user_info = std::make_shared<UserInfo>();
      user_info->name = res->getString("name");
      user_info->pwd = res->getString("pwd");
      user_info->email = res->getString("email");
      user_info->uid = res->getInt("uid");
      return user_info;
    }
  } catch (sql::SQLException &e) {
    LOG(warning) << "SQLException: " << e.what();
    LOG(warning) << " (MySQL error code: " << e.getErrorCode();
    LOG(warning) << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    return nullptr;
  }
}

std::shared_ptr<UserInfo> MysqlDao::GetUser(std::string name) {
  auto conn = _pool->GetConnection();

  try {
    if (conn == nullptr) {
      return nullptr;
    }

    std::unique_ptr<sql::PreparedStatement> pstmt(
        conn->_connection->prepareStatement(
            "SELECT name, pwd, email, uid FROM user WHERE name = ?"));
    pstmt->setString(1, name);

    std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
    if (res->next()) {
      auto user_info = std::make_shared<UserInfo>();
      user_info->name = res->getString("name");
      user_info->pwd = res->getString("pwd");
      user_info->email = res->getString("email");
      user_info->uid = res->getInt("uid");
      return user_info;
    }
  } catch (sql::SQLException &e) {
    LOG(warning) << "SQLException: " << e.what();
    LOG(warning) << " (MySQL error code: " << e.getErrorCode();
    LOG(warning) << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    return nullptr;
  }
}

bool MysqlDao::AddFriendApply(const int &from, const int &to) {
  auto conn = _pool->GetConnection();

  try {
    if (conn == nullptr) {
      return false;
    }

    std::unique_ptr<sql::PreparedStatement> pstmt(
        conn->_connection->prepareStatement(
            "INSERT INTO friend_apply (from_uid, to_uid) VALUES (?, ?)"));
    pstmt->setInt(1, from);
    pstmt->setInt(2, to);

    return pstmt->executeUpdate();
  } catch (sql::SQLException &e) {
    LOG(warning) << "SQLException: " << e.what();
    LOG(warning) << " (MySQL error code: " << e.getErrorCode();
    LOG(warning) << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    return false;
  }
}

bool MysqlDao::AuthFriendApply(const int &from, const int &to) {
  auto conn = _pool->GetConnection();

  try {
    if (conn == nullptr) {
      return false;
    }

    std::unique_ptr<sql::PreparedStatement> pstmt(
        conn->_connection->prepareStatement(
            "INSERT INTO friend (from_uid, to_uid) VALUES (?, ?)"));
    pstmt->setInt(1, from);
    pstmt->setInt(2, to);

    return pstmt->executeUpdate();
  } catch (sql::SQLException &e) {
    LOG(warning) << "SQLException: " << e.what();
    LOG(warning) << " (MySQL error code: " << e.getErrorCode();
    LOG(warning) << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    return false;
  }
}

bool MysqlDao::AddFriend(const int &from, const int &to,
                         std::string back_name) {
  auto conn = _pool->GetConnection();

  try {
    if (conn == nullptr) {
      return false;
    }

    // Begin transaction
    conn->_connection->setAutoCommit(false);

    // Insert both directions of friendship relation
    std::unique_ptr<sql::PreparedStatement> pstmt(
        conn->_connection->prepareStatement(
            "INSERT IGNORE INTO friend(self_id, friend_id, back) "
            "VALUES (?, ?, ?) "));

    // From user to friend
    pstmt->setInt(1, from);
    pstmt->setInt(2, to);
    pstmt->setString(3, back_name);
    int rowAffected = pstmt->executeUpdate();
    if (rowAffected < 0) {
      conn->_connection->rollback();
      return false;
    }

    // 准备第二个SQL语句，插入申请方好友数据
    std::unique_ptr<sql::PreparedStatement> pstmt2(
        conn->_connection->prepareStatement(
            "INSERT IGNORE INTO friend(self_id, friend_id, back) "
            "VALUES (?, ?, ?) "));
    // 反过来的申请时from，验证时to
    pstmt2->setInt(1, to); // from id
    pstmt2->setInt(2, from);
    pstmt2->setString(3, "");
    // 执行更新
    int rowAffected2 = pstmt2->executeUpdate();
    if (rowAffected2 < 0) {
      conn->_connection->rollback();
      return false;
    }

    // 提交事务
    conn->_connection->commit();
    LOG(info) << "addfriend insert friends success" << std::endl;

    return true;

  } catch (sql::SQLException &e) {
    // 如果发生错误，回滚事务
    if (conn) {
      conn->_connection->rollback();
    }
    LOG(warning) << "SQLException: " << e.what();
    LOG(warning) << " (MySQL error code: " << e.getErrorCode();
    LOG(warning) << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    return false;
  }

  return true;
}

bool MysqlDao::GetApplyList(int touid,
                            std::vector<std::shared_ptr<ApplyInfo>> &applyList,
                            int begin, int limit) {
  auto con = _pool->GetConnection();
  if (con == nullptr) {
    return false;
  }

  try {
    // 准备SQL语句, 根据起始id和限制条数返回列表
    std::unique_ptr<sql::PreparedStatement> pstmt(
        con->_connection->prepareStatement(
            "select apply.from_uid, apply.status, user.name, "
            "user.nick, user.sex from friend_apply as apply join user on "
            "apply.from_uid = user.uid where apply.to_uid = ? "
            "and apply.id > ? order by apply.id ASC LIMIT ? "));

    pstmt->setInt(1, touid); // 将uid替换为你要查询的uid
    pstmt->setInt(2, begin); // 起始id
    pstmt->setInt(3, limit); // 偏移量
    // 执行查询
    std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
    // 遍历结果集
    while (res->next()) {
      auto name = res->getString("name");
      auto uid = res->getInt("from_uid");
      auto status = res->getInt("status");
      auto nick = res->getString("nick");
      auto sex = res->getInt("sex");
      auto apply_ptr = std::make_shared<ApplyInfo>(
          uid, std::string(name), "", "", std::string(nick), sex, status);
    }
    return true;
  } catch (sql::SQLException &e) {
    LOG(warning) << "SQLException: " << e.what();
    LOG(warning) << " (MySQL error code: " << e.getErrorCode();
    LOG(warning) << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    return false;
  }
}

bool MysqlDao::GetFriendList(
    int self_id, std::vector<std::shared_ptr<UserInfo>> &user_info_list) {

  auto con = _pool->GetConnection();
  if (con == nullptr) {
    return false;
  }

  try {
    // 准备SQL语句, 根据起始id和限制条数返回列表
    std::unique_ptr<sql::PreparedStatement> pstmt(
        con->_connection->prepareStatement("select * from friend where self_id = ? "));

    pstmt->setInt(1, self_id); // 将uid替换为你要查询的uid

    // 执行查询
    std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
    // 遍历结果集
    while (res->next()) {
      auto friend_id = res->getInt("friend_id");
      auto back = res->getString("back");
      // 再一次查询friend_id对应的信息
      auto user_info = GetUser(friend_id);
      if (user_info == nullptr) {
        continue;
      }

      user_info->back = user_info->name;
      user_info_list.push_back(user_info);
    }
    return true;
  } catch (sql::SQLException &e) {
    LOG(warning) << "SQLException: " << e.what();
    LOG(warning) << " (MySQL error code: " << e.getErrorCode();
    LOG(warning) << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    return false;
  }

  return true;
}