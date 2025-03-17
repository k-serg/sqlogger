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

#ifndef LOG_WRITER_H
#define LOG_WRITER_H

#include "log_entry.h"
#include "database_interface.h"

/**
 * @class LogWriter
 * @brief Class for writing log entries to a database.
 */
class LogWriter
{
    public:
        /**
         * @brief Constructs a LogWriter object.
         * @param database The database interface to use for writing logs.
         */
        LogWriter(IDatabase& database) : database(database) {}

        /**
         * @brief Writes a log entry to the database.
         * @param entry The log entry to write.
         * @return True if the log entry was written successfully, false otherwise.
         */
        bool writeLog(const LogEntry& entry);

        /**
         * @brief Clears all log entries from the database.
         */
        void clearLogs();

#ifdef USE_SOURCE_INFO
        /**
        * @brief Clears all source entries from the database.
        */
        void clearSources();
#endif

        /**
         * @brief Creates the log table in the database if it does not exist.
         */
        void createLogsTable();

        /**
         * @brief Creates indexes on the log table for faster queries.
         */
        void createIndexes();

#ifdef USE_SOURCE_INFO
        /**
         * @brief Creates the sources table in the database if it does not exist.
         * This method creates a table for storing source information (e.g., UUID, name) in the database.
         * If the table already exists, it will not be recreated.
         */
        void LogWriter::createSourcesTable();
#endif

#ifdef USE_SOURCE_INFO
        /**
        * @brief Adds a new source to the database.
        * @param name The name of the source.
        * @param uuid The UUID of the source.
        * @return The ID of the newly added source, or SOURCE_NOT_FOUND if the operation failed.
        */
        int LogWriter::addSource(const std::string& name, const std::string& uuid = "");
#endif

    private:
        IDatabase& database; /**< The database interface used for writing logs. */
};

#endif // LOG_WRITER_H