/*
 * This file is part of SQLogger.
 *
 * SQLogger is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SQLogger is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SQLogger. If not, see <https://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2025 Sergey K. sergey[no_spam]@greenblit.com
 */

#ifndef DATABASE_FACTORY_H
#define DATABASE_FACTORY_H

#include <memory>
#include "database_interface.h"
#include "database_helper.h"
#include "mock_database.h"
#include "sqlite_database.h"

#ifdef USE_MYSQL
    #include "mysql_database.h"
#endif

#ifdef USE_POSTGRESQL
    #include "postgresql_database.h"
#endif

#ifdef USE_MONGODB
    #include "mongodb_database.h"
#endif

class DatabaseFactory
{
    public:
        static std::unique_ptr<IDatabase> create(DataBaseType type, const std::string& connectionString);
};

#endif // !DATABASE_FACTORY_H