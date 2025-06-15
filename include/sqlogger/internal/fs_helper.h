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

#ifndef FS_HELPER_H
#define FS_HELPER_H

#include <filesystem>
#include <string>
#include <iostream>
#include <sqlogger/internal/log_strings.h>

// Constants
#define ERR_LOG_FILE "error_log.txt"
#define MAX_ERROR_LOG_SIZE 10.0 // Megabytes

/**
 * @class FSHelper
 * @brief Provides file system helper operations
 */
namespace FSHelper
{
    /**
     * @brief Create directories from path.
     * @param path Path to the file/directory.
     * @param errMsg[out] Error message.
     * If path contains filename it will be removed.
     * @return bool True, if path created or already exists.
     * False otherwise.
    */
    bool createDir(const std::string& path, std::string& errMsg = std::string());

    /**
     * @brief Deletes a file at the specified path
     * @param path The filesystem path of the file to delete
     * @param errMsg[out] Reference to string that will contain error message if operation fails
     * @return true if file was successfully deleted
     * @return false if file didn't exist or couldn't be deleted (check errMsg for details)
     */
    bool deleteFile(const std::string& path, std::string& errMsg);

    /**
     * @brief Extracts the filename component from a path
     * @param path The full filesystem path
     * @return std::string Just the filename portion of the path
     */
    std::string toFilename(const std::string& path);

    /**
     * @brief Gets the size of a file in megabytes
     * @param path The filesystem path of the file to check
     * @return double File size in megabytes (MB), or 0.0 if size couldn't be determined
     */
    double fileSize(const std::string& path);

    /**
     * @brief Checks if a log file needs rotation based on its size
     * @param path The filesystem path of the log file to check
     * @return true if file size exceeds MAX_ERROR_LOG_SIZE
     * @return false if file size is within limits or couldn't be checked
     */
    bool needLogRotation(const std::string& path);

    /**
     * @brief Performs log rotation by deleting the specified log file
     * @param path The filesystem path of the log file to rotate
     * @return true if log file was successfully deleted
     * @return false if deletion failed (error will be printed to stderr)
     */
    bool rotateLog(const std::string& path);

};

#endif // !FS_HELPER_H
