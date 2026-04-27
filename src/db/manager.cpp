#include "db/manager.hpp"

namespace lfw::db
{

DbManager::DbManager()
{
    config->path_to_database = "librefw.db";
    config->flags = SQLITE_OPEN_READWRITE;
    db.connect_using(config);
}

} // namespace lfw::db
