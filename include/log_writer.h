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

//using namespace LogHelper;

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

        /**
         * @brief Creates the log table in the database if it does not exist.
         */
        void createTable();

        /**
         * @brief Creates indexes on the log table for faster queries.
         */
        void createIndexes();

    private:
        IDatabase& database; /**< The database interface used for writing logs. */
};

#endif // LOG_WRITER_H