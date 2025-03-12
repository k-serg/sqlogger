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
                        + std::string(FIELD_TIMESTAMP) + ", "
                        + std::string(FIELD_LEVEL) + ", "
                        + std::string(FIELD_MESSAGE) + ", "
                        + std::string(FIELD_FUNCTION) + ", "
                        + std::string(FIELD_FILE) + ", "
                        + std::string(FIELD_LINE) + ", "
                        + std::string(FIELD_THREAD_ID) + ") "
                        "VALUES (?, ?, ?, ?, ?, ?, ?);";

    std::vector<std::string> params =
    {
        entry.timestamp,
        entry.level,
        entry.message,
        entry.function,
        entry.file,
        std::to_string(entry.line),
        entry.threadId
    };

    return database.executeWithParams(query, params);
}

/**
 * @brief Clears all log entries from the database.
 */
void LogWriter::clearLogs()
{
    database.execute("DELETE FROM " + std::string(LOG_TABLE_NAME) + ";");
}

/**
 * @brief Creates the log table in the database if it does not exist.
 */
void LogWriter::createTable()
{
    std::string createTableQuery;

    switch(database.getDatabaseType())
    {
        case DataBaseType::SQLite:
            createTableQuery = "CREATE TABLE IF NOT EXISTS " + std::string(LOG_TABLE_NAME) + " ("
                               + std::string(FIELD_ID) + " INTEGER PRIMARY KEY AUTOINCREMENT, "
                               + std::string(FIELD_TIMESTAMP) + " DATETIME NOT NULL, "
                               + std::string(FIELD_LEVEL) + " TEXT NOT NULL, "
                               + std::string(FIELD_MESSAGE) + " TEXT NOT NULL, "
                               + std::string(FIELD_FUNCTION) + " TEXT NOT NULL, "
                               + std::string(FIELD_FILE) + " TEXT NOT NULL, "
                               + std::string(FIELD_LINE) + " INTEGER NOT NULL, "
                               + std::string(FIELD_THREAD_ID) + " TEXT NOT NULL);";
            break;
        case DataBaseType::MySQL:
            createTableQuery = "CREATE TABLE IF NOT EXISTS " + std::string(LOG_TABLE_NAME) + " ("
                               + std::string(FIELD_ID) + " INTEGER PRIMARY KEY AUTO_INCREMENT, "
                               + std::string(FIELD_TIMESTAMP) + " DATETIME NOT NULL, "
                               + std::string(FIELD_LEVEL) + " TEXT NOT NULL, "
                               + std::string(FIELD_MESSAGE) + " TEXT NOT NULL, "
                               + std::string(FIELD_FUNCTION) + " TEXT NOT NULL, "
                               + std::string(FIELD_FILE) + " TEXT NOT NULL, "
                               + std::string(FIELD_LINE) + " INTEGER NOT NULL, "
                               + std::string(FIELD_THREAD_ID) + " TEXT NOT NULL);";
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
                "CREATE INDEX IF NOT EXISTS idx_" + std::string(FIELD_TIMESTAMP) + " ON " + std::string(LOG_TABLE_NAME) + " (" + FIELD_TIMESTAMP + ");",
                "CREATE INDEX IF NOT EXISTS idx_" + std::string(FIELD_LEVEL) + " ON " + std::string(LOG_TABLE_NAME) + " (" + FIELD_LEVEL + ");",
                "CREATE INDEX IF NOT EXISTS idx_" + std::string(FIELD_FILE) + " ON " + std::string(LOG_TABLE_NAME) + " (" + FIELD_FILE + ");",
                "CREATE INDEX IF NOT EXISTS idx_" + std::string(FIELD_THREAD_ID) + " ON " + std::string(LOG_TABLE_NAME) + " (" + FIELD_THREAD_ID + ");",
                "CREATE INDEX IF NOT EXISTS idx_" + std::string(FIELD_FUNCTION) + " ON " + std::string(LOG_TABLE_NAME) + " (" + FIELD_FUNCTION + ");"
            };
            break;

        case DataBaseType::MySQL:
            indexQueries =
            {
                "ALTER TABLE " + std::string(LOG_TABLE_NAME) + " ADD INDEX idx_" + FIELD_TIMESTAMP + " (" + FIELD_TIMESTAMP + ");",
                "ALTER TABLE " + std::string(LOG_TABLE_NAME) + " ADD INDEX idx_" + FIELD_LEVEL + " (" + FIELD_LEVEL + ");",
                "ALTER TABLE " + std::string(LOG_TABLE_NAME) + " ADD INDEX idx_" + FIELD_FILE + " (" + FIELD_FILE + ");",
                "ALTER TABLE " + std::string(LOG_TABLE_NAME) + " ADD INDEX idx_" + FIELD_THREAD_ID + " (" + FIELD_THREAD_ID + ");",
                "ALTER TABLE " + std::string(LOG_TABLE_NAME) + " ADD INDEX idx_" + FIELD_FUNCTION + " (" + FIELD_FUNCTION + ");"
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
