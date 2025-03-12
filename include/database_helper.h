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

#ifndef DATABASE_HELPER_H
#define DATABASE_HELPER_H

#include <string>
#include "log_entry.h"

#define DB_TYPE_STR_MOCK "Mock"
#define DB_TYPE_STR_SQLITE "SQLite"
#define DB_TYPE_STR_MYSQL "MySQL"
#define DB_TYPE_STR_UNKNOWN "UNKNOWN"

/**
 * @enum DataBaseType
 * @brief Enumeration representing the type of database.
 */
enum class DataBaseType
{
    Unknown = -1, /**< Unknown database type. */
    Mock,         /**< Mock database type. */
    SQLite,       /**< SQLite database. */
    MySQL         /**< MySQL database. */
};

namespace DataBaseHelper
{

    /**
     * @brief Converts a string representation of a database type to the corresponding enum value.
     * @param stringType The string representation of the database type.
     * @return The corresponding DataBaseType enum value.
     */
    static DataBaseType stringToDatabaseType(const std::string& stringType)
    {
        if(LogHelper::toUpperCase(stringType) == LogHelper::toUpperCase(DB_TYPE_STR_MOCK)) return DataBaseType::Mock;
        if(LogHelper::toUpperCase(stringType) == LogHelper::toUpperCase(DB_TYPE_STR_SQLITE)) return DataBaseType::SQLite;
        if(LogHelper::toUpperCase(stringType) == LogHelper::toUpperCase(DB_TYPE_STR_MYSQL)) return DataBaseType::MySQL;
        return DataBaseType::Unknown;
    };

    /**
     * @brief Converts a DataBaseType enum value to its string representation.
     * @param type The DataBaseType enum value.
     * @return The string representation of the database type.
     */
    static std::string databaseTypeToString(const DataBaseType& type)
    {
        switch(type)
        {
            case DataBaseType::Mock:
                return DB_TYPE_STR_MOCK;
                break;
            case DataBaseType::SQLite:
                return DB_TYPE_STR_SQLITE;
                break;
            case DataBaseType::MySQL:
                return DB_TYPE_STR_MYSQL;
                break;
            default:
                return DB_TYPE_STR_UNKNOWN;
                break;
        }
    };

    /**
     * @brief Escapes backslashes in a string.
     * @param input The input string to escape.
     * @return The string with escaped backslashes.
     */
    static std::string escapeBackslashes(const std::string& input)
    {
        std::string result;
        for(size_t i = 0; i < input.size(); ++i)
        {
            if(input[i] == '\\')
            {
                if(i + 1 < input.size() && input[i + 1] == '\\')
                {
                    result += '\\';
                    ++i;
                }
                else
                {
                    result += '\\';
                }
            }
            result += input[i];
        }
        return result;
    }
};

#endif // !DATABASE_HELPER_H
