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
    std::string createTableQuery = "CREATE TABLE IF NOT EXISTS " + std::string(LOG_TABLE_NAME) + " ("
                                   + std::string(FIELD_ID) + " INTEGER PRIMARY KEY AUTOINCREMENT, "
                                   + std::string(FIELD_TIMESTAMP) + " DATETIME NOT NULL, "
                                   + std::string(FIELD_LEVEL) + " TEXT NOT NULL, "
                                   + std::string(FIELD_MESSAGE) + " TEXT NOT NULL, "
                                   + std::string(FIELD_FUNCTION) + " TEXT NOT NULL, "
                                   + std::string(FIELD_FILE) + " TEXT NOT NULL, "
                                   + std::string(FIELD_LINE) + " INTEGER NOT NULL, "
                                   + std::string(FIELD_THREAD_ID) + " TEXT NOT NULL);";

    database.execute(createTableQuery);
}

/**
 * @brief Creates indexes on the log table for faster queries.
 */
void LogWriter::createIndexes()
{
    std::string createIndexQuery =
        "CREATE INDEX IF NOT EXISTS idx_" + std::string(FIELD_TIMESTAMP) + " ON " + std::string(LOG_TABLE_NAME) + " (" + FIELD_TIMESTAMP + ");" + "\n"
        + "CREATE INDEX IF NOT EXISTS idx_" + std::string(FIELD_LEVEL) + " ON " + std::string(LOG_TABLE_NAME) + " (" + FIELD_LEVEL + ");" + "\n"
        + "CREATE INDEX IF NOT EXISTS idx_" + std::string(FIELD_FILE) + " ON " + std::string(LOG_TABLE_NAME) + " (" + FIELD_FILE + ");" + "\n"
        + "CREATE INDEX IF NOT EXISTS idx_" + std::string(FIELD_THREAD_ID) + " ON " + std::string(LOG_TABLE_NAME) + " (" + FIELD_THREAD_ID + ");" + "\n"
        + "CREATE INDEX IF NOT EXISTS idx_" + std::string(FIELD_TIMESTAMP) + " ON " + std::string(LOG_TABLE_NAME) + " (" + FIELD_TIMESTAMP + ");" + "\n"
        + "CREATE INDEX IF NOT EXISTS idx_" + std::string(FIELD_FUNCTION) + " ON " + std::string(LOG_TABLE_NAME) + " (" + FIELD_FUNCTION + ");";
    database.execute(createIndexQuery);
}