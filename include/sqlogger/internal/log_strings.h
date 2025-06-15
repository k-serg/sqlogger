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
#define ERR_MSG_FAILED_CREATE_DB "Failed to create database: "
#define ERR_MSG_FAILED_OPEN_DB "Failed to open database: "
#define ERR_MSG_FAILED_NOT_CONNECTED_DB "Not connected to database"
#define ERR_MSG_FAILED_OPEN_FILE "Failed to open file: "
#define ERR_MSG_FAILED_OPEN_FILE_RW "Failed to open file for writing: "
#define ERR_MSG_FAILED_ENABLE_WAL "Failed to enable WAL mode"
#define ERR_MSG_SQL_ERR "SQL error: "
#define ERR_MSG_FAILED_PREPARE_STMT "Failed to prepare statement: "
#define ERR_MSG_FAILED_RECONNECT_DB "Failed to reconnect to database"
#define ERR_MSG_UNKNOWN_EXPORT_FMT "Unknown export format"
#define ERR_MSG_CONNECTION_FAILED "Connection failed: "
#define ERR_MSG_MYSQL_INIT_FAILED "MySQL initialization failed"
#define ERR_MSG_DROP_NOT_ALLOWED "Database drop is not allowed"
#define ERR_MSG_UNSUPPORTED_DB "Unsupported database type"
#define ERR_MSG_PASSKEY_EMPTY "Passkey is empty. Set passkey value"
#define ERR_MSG_FAILED_BATCH_TASK "Batch task failed: "
#define ERR_MSG_FAILED_BATCH_QUERY "Batch query failed: "
#define ERR_MSG_LOGGER_EXISTS "Logger exists: "
#define ERR_MSG_DB_TYPE_NOT_SPECIFIED "Database type not specified"
#define ERR_MSG_LOGGER_NAME_NOT_FOUND "Logger name not found: "
#define ERR_MSG_AVAILABLE_LOGGERS "Available loggers: "
#define ERR_MSG_UUID_NOT_CORRECT "UUID is not correct: "
#define ERR_MSG_DANGEROUS_SQL_PAT "Contains dangerous SQL pattern"
#define ERR_MSG_NOT_SUPPORTED_BUILD "not supported in this build"
#define ERR_MSG_MISSING_PARAMS "Missing params: "
#define ERR_MSG_INVALID_PARAMS "Invalid params: "
#define ERR_MSG_INVALID_OPERATOR "Invalid filter operator: "
#define ERR_MSG_FILTER_OP_EMPTY "Filter operator cannot be empty"
#define ERR_MSG_UNABLE_DELETE_ERRLOG "Unable to delete error log file: "
#define ERR_MSG_DELETED_FILE_NOT_EXISTS "File to delete not exists: "
#define ERR_MSG_UNABLE_OBTAIN_FILESIZE "Unable to obtain file size: "

#ifdef SQLG_USE_AES
    #define ERR_MSG_CRYPTO_ENC_INIT_FAILED "Encryption init failed"
    #define ERR_MSG_CRYPTO_ENC_UPDATE_FAILED "Encryption update failed"
    #define ERR_MSG_CRYPTO_ENC_FINAL_FAILED "Encryption final failed"
    #define ERR_MSG_CRYPTO_DEC_INIT_FAILED "Decryption init failed"
    #define ERR_MSG_CRYPTO_DEC_UPDATE_FAILED "Decryption update failed"
    #define ERR_MSG_CRYPTO_DEC_FINAL_FAILED "Decryption final failed"
    #define ERR_MSG_CRYPTO_EVPCON_FAILED "Failed to create EVP context"
#endif

#ifdef SQLG_USE_SOURCE_INFO
    #define ERR_MSG_SOURCE_NOT_FOUND "Source info not found for sourceId: "
    #define ERR_MSG_SOURCE_ID_NOT_INIT "sourceId is not initialized. Cannot log message."
    #define ERR_MSG_FAILED_TO_ADD_SOURCE "Failed to add source to the database: "
#endif

#endif // !LOG_STRINGS_H
