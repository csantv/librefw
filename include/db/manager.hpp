#pragma once

#include <sqlpp23/sqlite3/database/connection.h>

#include <memory>
#include <mutex>
#include <type_traits>

namespace lfw::db
{

using connection = sqlpp::sqlite3::connection;
using connection_config = sqlpp::sqlite3::connection_config;

class DbManager
{
public:
    DbManager();

    template <typename F>
    auto execute(F&& func) -> std::invoke_result_t<F, connection&>
    {
        std::scoped_lock lock {db_mutex};
        return std::forward<F>(func)(db);
    }

private:
    connection db;
    std::shared_ptr<connection_config> config {std::make_shared<connection_config>()};

    std::mutex db_mutex;  // needed for sqlite
};

}
