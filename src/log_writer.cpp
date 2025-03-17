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

#include "log_writer.h"

/**
 * @brief Writes a log entry to the database.
 * @param entry The log entry to write.
 * @return True if the log entry was written successfully, false otherwise.
 */
bool LogWriter::writeLog(const LogEntry& entry)
{
    std::string query = "INSERT INTO " + std::string(LOG_TABLE_NAME) + " ("
#ifdef USE_SOURCE_INFO
                        + std::string(FIELD_LOG_SOURCES_ID) + ", "
#endif
                        + std::string(FIELD_LOG_TIMESTAMP) + ", "
                        + std::string(FIELD_LOG_LEVEL) + ", "
                        + std::string(FIELD_LOG_MESSAGE) + ", "
                        + std::string(FIELD_LOG_FUNCTION) + ", "
                        + std::string(FIELD_LOG_FILE) + ", "
                        + std::string(FIELD_LOG_LINE) + ", "
                        + std::string(FIELD_LOG_THREAD_ID)
                        + ") VALUES ("
#ifdef USE_SOURCE_INFO
                        "?, "
#endif
                        "?, ?, ?, ?, ?, ?, ?);";

    std::vector<std::string> params;
#ifdef USE_SOURCE_INFO
    params.push_back(std::to_string(entry.sourceId));
#endif
    params.push_back(entry.timestamp);
    params.push_back(entry.level);
    params.push_back(entry.message);
    params.push_back(entry.function);
    params.push_back(entry.file);
    params.push_back(std::to_string(entry.line));
    params.push_back(entry.threadId);
    return database.executeWithParams(query, params);
}

/**
 * @brief Clears all log entries from the database.
 */
void LogWriter::clearLogs()
{
    database.execute("DELETE FROM " + std::string(LOG_TABLE_NAME) + ";");
}

#ifdef USE_SOURCE_INFO
/**
* @brief Clears all source entries from the database.
*/
void LogWriter::clearSources()
{
    database.execute("DELETE FROM " + std::string(SOURCES_TABLE_NAME) + ";");
}
#endif

/**
 * @brief Creates the log table in the database if it does not exist.
 */
void LogWriter::createLogsTable()
{
    std::string createTableQuery;

    switch(database.getDatabaseType())
    {
        case DataBaseType::SQLite:
            createTableQuery = "CREATE TABLE IF NOT EXISTS " + std::string(LOG_TABLE_NAME) + " ("
                               + std::string(FIELD_LOG_ID) + " INTEGER PRIMARY KEY AUTOINCREMENT, "
#ifdef USE_SOURCE_INFO
                               + std::string(FIELD_LOG_SOURCES_ID) + " INT, "
#endif
                               + std::string(FIELD_LOG_TIMESTAMP) + " DATETIME NOT NULL, "
                               + std::string(FIELD_LOG_LEVEL) + " TEXT NOT NULL, "
                               + std::string(FIELD_LOG_MESSAGE) + " TEXT NOT NULL, "
                               + std::string(FIELD_LOG_FUNCTION) + " TEXT NOT NULL, "
                               + std::string(FIELD_LOG_FILE) + " TEXT NOT NULL, "
                               + std::string(FIELD_LOG_LINE) + " INTEGER NOT NULL, "
                               + std::string(FIELD_LOG_THREAD_ID) + " TEXT NOT NULL"
#ifdef USE_SOURCE_INFO
                               + ", FOREIGN KEY (" + std::string(FIELD_LOG_SOURCES_ID) + ") REFERENCES " + std::string(SOURCES_TABLE_NAME) + "(" + std::string(FIELD_SOURCES_ID) + ")"
#endif
                               + ");";
            break;

        case DataBaseType::MySQL:
            createTableQuery = "CREATE TABLE IF NOT EXISTS " + std::string(LOG_TABLE_NAME) + " ("
                               + std::string(FIELD_LOG_ID) + " BIGINT PRIMARY KEY AUTO_INCREMENT, "
#ifdef USE_SOURCE_INFO
                               + std::string(FIELD_LOG_SOURCES_ID) + " INT, "
#endif
                               + std::string(FIELD_LOG_TIMESTAMP) + " DATETIME NOT NULL, "
                               + std::string(FIELD_LOG_LEVEL) + " TEXT NOT NULL, "
                               + std::string(FIELD_LOG_MESSAGE) + " TEXT NOT NULL, "
                               + std::string(FIELD_LOG_FUNCTION) + " TEXT NOT NULL, "
                               + std::string(FIELD_LOG_FILE) + " TEXT NOT NULL, "
                               + std::string(FIELD_LOG_LINE) + " INTEGER NOT NULL, "
                               + std::string(FIELD_LOG_THREAD_ID) + " TEXT NOT NULL"
#ifdef USE_SOURCE_INFO
                               + ", FOREIGN KEY (" + std::string(FIELD_LOG_SOURCES_ID) + ") REFERENCES " + std::string(SOURCES_TABLE_NAME) + "(" + std::string(FIELD_SOURCES_ID) + ")"
#endif
                               + ");";
            break;

        default:
            break;
    }

    database.execute(createTableQuery);
}

/**
 * @brief Creates indexes on the log table for faster queries.
 */
void LogWriter::createIndexes()
{
    std::vector<std::string> indexQueries;

    switch(database.getDatabaseType())
    {
        case DataBaseType::SQLite:
            indexQueries =
            {
                "CREATE INDEX IF NOT EXISTS idx_" + std::string(FIELD_LOG_TIMESTAMP) + " ON " + std::string(LOG_TABLE_NAME) + " (" + FIELD_LOG_TIMESTAMP + ");",
                "CREATE INDEX IF NOT EXISTS idx_" + std::string(FIELD_LOG_LEVEL) + " ON " + std::string(LOG_TABLE_NAME) + " (" + FIELD_LOG_LEVEL + ");",
                "CREATE INDEX IF NOT EXISTS idx_" + std::string(FIELD_LOG_FILE) + " ON " + std::string(LOG_TABLE_NAME) + " (" + FIELD_LOG_FILE + ");",
                "CREATE INDEX IF NOT EXISTS idx_" + std::string(FIELD_LOG_THREAD_ID) + " ON " + std::string(LOG_TABLE_NAME) + " (" + FIELD_LOG_THREAD_ID + ");",
                "CREATE INDEX IF NOT EXISTS idx_" + std::string(FIELD_LOG_FUNCTION) + " ON " + std::string(LOG_TABLE_NAME) + " (" + FIELD_LOG_FUNCTION + ");"
            };
            break;

        case DataBaseType::MySQL:
            indexQueries =
            {
                "ALTER TABLE " + std::string(LOG_TABLE_NAME) + " ADD INDEX idx_" + FIELD_LOG_TIMESTAMP + " (" + FIELD_LOG_TIMESTAMP + ");",
                "ALTER TABLE " + std::string(LOG_TABLE_NAME) + " ADD INDEX idx_" + FIELD_LOG_LEVEL + " (" + FIELD_LOG_LEVEL + ");",
                "ALTER TABLE " + std::string(LOG_TABLE_NAME) + " ADD INDEX idx_" + FIELD_LOG_FILE + " (" + FIELD_LOG_FILE + ");",
                "ALTER TABLE " + std::string(LOG_TABLE_NAME) + " ADD INDEX idx_" + FIELD_LOG_THREAD_ID + " (" + FIELD_LOG_THREAD_ID + ");",
                "ALTER TABLE " + std::string(LOG_TABLE_NAME) + " ADD INDEX idx_" + FIELD_LOG_FUNCTION + " (" + FIELD_LOG_FUNCTION + ");"
            };
            break;

        default:
            break;
    }

    for(const auto & query : indexQueries)
    {
        if(database.getDatabaseType() == DataBaseType::MySQL)
        {
            std::string checkQuery =
                "SELECT COUNT(*) "
                "FROM information_schema.statistics "
                "WHERE table_schema = DATABASE() "
                "  AND table_name = '" + std::string(LOG_TABLE_NAME) + "' "
                "  AND index_name = 'idx_" + query.substr(query.find("idx_") + 4, query.find(" ") - query.find("idx_") - 4) + "';";

            auto result = database.query(checkQuery);
            if(!result.empty() && result[0].at("COUNT(*)") != "0")
            {
                // Index already exists, skip.
                continue;
            }
        }

        database.execute(query);
    }
}

#ifdef USE_SOURCE_INFO
/**
 * @brief Creates the sources table in the database if it does not exist.
 * This method creates a table for storing source information (e.g., UUID, name) in the database.
 * If the table already exists, it will not be recreated.
 */
void LogWriter::createSourcesTable()
{
    std::string createTableQuery;

    switch(database.getDatabaseType())
    {
        case DataBaseType::SQLite:
            createTableQuery = "CREATE TABLE IF NOT EXISTS " + std::string(SOURCES_TABLE_NAME) + " ("
                               + std::string(FIELD_SOURCES_ID) + " INTEGER PRIMARY KEY AUTOINCREMENT, "
                               + std::string(FIELD_SOURCES_UUID) + " CHAR(36) UNIQUE NOT NULL, "
                               + std::string(FIELD_SOURCES_NAME) + " TEXT NOT NULL);";
            break;

        case DataBaseType::MySQL:
            createTableQuery = "CREATE TABLE IF NOT EXISTS " + std::string(SOURCES_TABLE_NAME) + " ("
                               + std::string(FIELD_SOURCES_ID) + " INTEGER PRIMARY KEY AUTO_INCREMENT, "
                               + std::string(FIELD_SOURCES_UUID) + " CHAR(36) UNIQUE NOT NULL, "
                               + std::string(FIELD_SOURCES_NAME) + " TEXT NOT NULL);";
            break;

        default:
            break;
    }

    database.execute(createTableQuery);
}
#endif

#ifdef USE_SOURCE_INFO
/**
 * @brief Adds a new source to the database.
 * @param name The name of the source.
 * @param uuid The UUID of the source.
 * @return The ID of the newly added source, or SOURCE_NOT_FOUND if the operation failed.
 */
int LogWriter::addSource(const std::string& name, const std::string& uuid)
{
    std::string query = "INSERT INTO " + std::string(SOURCES_TABLE_NAME) + " ("
                        + std::string(FIELD_SOURCES_UUID) + ", "
                        + std::string(FIELD_SOURCES_NAME) + ") VALUES (?, ?);";

    std::vector<std::string> params =
    {
        uuid.empty() ? LogHelper::generateUUID() : uuid, name
    };

    if(database.executeWithParams(query, params))
    {
        std::vector<std::map<std::string, std::string>> result;

        switch(database.getDatabaseType())
        {
            case DataBaseType::Mock:
                return 0;
                break;
            case DataBaseType::SQLite:
                result = database.query("SELECT LAST_INSERT_ROWID();");
                if(!result.empty())
                {
                    return std::stoi(result[0].at("LAST_INSERT_ROWID()"));
                }
                break;
            case DataBaseType::MySQL:
                result = database.query("SELECT LAST_INSERT_ID();");
                if(!result.empty())
                {
                    return std::stoi(result[0].at("LAST_INSERT_ID()"));
                }
                break;
            default:
                break;
        }

    }
    return -1;
}
#endif
