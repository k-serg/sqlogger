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

#ifndef LOG_STRINGS_H
#define LOG_STRINGS_H

#define ERR_MSG_FAILED_QUERY "Failed to execute query"
#define ERR_MSG_FAILED_TASK "Error in processTask: "
#define ERR_MSG_FAILED_OPEN_ERR_LOG "Failed to open error log file: "
#define ERR_MSG_TIMEOUT_TASK_QUEUE "Timeout while waiting for task queue to empty"
#define ERR_MSG_FAILED_CREATE_DIR "Failed to create directory: "
#define ERR_MSG_FAILED_OPEN_DB "Failed to open database: "
#define ERR_MSG_FAILED_OPEN_FILE "Failed to open file: "
#define ERR_MSG_FAILED_OPEN_FILE_RW "Failed to open file for writing: "
#define ERR_MSG_FAILED_ENABLE_WAL "Failed to enable WAL mode"
#define ERR_MSG_SQL_ERR "SQL error: "
#define ERR_MSG_FAILED_PREPARE_STMT "Failed to prepare statement: "
#define ERR_MSG_FAILED_RECONNECT_DB "Failed to reconnect to database"
#define ERR_MG_UNKNOWN_EXPORT_FMT "Unknown export format"

#endif // !LOG_STRINGS_H
