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

#include "database_factory.h"

 /**
  * @brief Creates a database instance of the specified type
  * @param type The type of database to create (see DataBaseType enum)
  * @param connectionString Connection string/parameters for the database
  * @return std::unique_ptr<IDatabase> Pointer to the created database instance
  *
  * @throws std::invalid_argument If unsupported database type is requested
  *
  * @example
  * // Create SQLite database
  * auto db = DatabaseFactory::create(DataBaseType::SQLite, "logs.db");
  *
  * // Create mock database (for testing)
  * auto mockDb = DatabaseFactory::create(DataBaseType::Mock);
  *
  * @note The actual available database types depend on compile-time definitions:
  *  - USE_MYSQL for MySQL support
  *  - USE_POSTGRESQL for PostgreSQL support
  *
  * @warning Connection string format is database-specific
  * @see DataBaseType for supported database types
  */
std::unique_ptr<IDatabase> DatabaseFactory::create(DataBaseType type, const std::string& connectionString)
{
    switch(type)
    {
        case DataBaseType::Mock:
            return std::make_unique<MockDatabase>();

        case DataBaseType::SQLite:
            return std::make_unique<SQLiteDatabase>(connectionString);

#ifdef USE_MYSQL
        case DataBaseType::MySQL:
            return std::make_unique<MySQLDatabase>(connectionString);
#endif

#ifdef USE_POSTGRESQL
        case DataBaseType::PostgreSQL:
            return std::make_unique<PostgreSQLDatabase>(connectionString);
#endif

#ifdef USE_MONGODB
        case DataBaseType::MongoDB:
            return std::make_unique<MongoDBDatabase>(connectionString);
#endif

        case DataBaseType::Unknown:
        default:
        {
            throw std::invalid_argument(ERR_MSG_UNSUPPORTED_DB);
        }
    }
}