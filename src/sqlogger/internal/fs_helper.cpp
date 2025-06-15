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

#include "sqlogger/internal/fs_helper.h"

/**
 * @brief Create directories from path.
 * @param path Path to the file/directory.
 * @param errMsg[out] Error message.
 * If path contains filename it will be removed.
 * @return bool True, if path created or already exists.
 * False otherwise.
*/

bool FSHelper::createDir(const std::string& path, std::string& errMsg)
{
    std::filesystem::path dir(path);
    dir = std::filesystem::absolute(dir);
    dir = dir.remove_filename();
    if(std::filesystem::exists(dir))
        return true;
    try
    {
        // May return false, if path contains trailing separatror. So we handle exception on fault, not bool result.
        std::filesystem::create_directories(dir);
    }
    catch(const std::exception& e)
    {
        errMsg = e.what();
        return false;
    }
    return true;
}

/**
 * @brief Deletes a file at the specified path
 * @param path The filesystem path of the file to delete
 * @param errMsg[out] Reference to string that will contain error message if operation fails
 * @return true if file was successfully deleted
 * @return false if file didn't exist or couldn't be deleted (check errMsg for details)
 */
bool FSHelper::deleteFile(const std::string& path, std::string& errMsg)
{
    std::filesystem::path file(path);
    file = std::filesystem::absolute(file);
    if(!std::filesystem::exists(file))
    {
        errMsg = ERR_MSG_DELETED_FILE_NOT_EXISTS + path;
        return false;
    }

    try
    {
        std::filesystem::remove(file);
    }
    catch(const std::exception& e)
    {
        errMsg = e.what();
        return false;
    }
    return true;
}

/**
 * @brief Extracts the filename component from a path
 * @param path The full filesystem path
 * @return std::string Just the filename portion of the path
 */
std::string FSHelper::toFilename(const std::string& path)
{
    return std::filesystem::path(path).filename().string();
}

/**
 * @brief Gets the size of a file in megabytes
 * @param path The filesystem path of the file to check
 * @return double File size in megabytes (MB), or 0.0 if size couldn't be determined
 */
double FSHelper::fileSize(const std::string& path)
{
    double size = 0.0;
    try
    {
        size = std::filesystem::file_size(path) / (static_cast<double>(1024) * 1024);
    }
    catch(const std::exception& e)
    {
        std::cerr << ERR_MSG_UNABLE_OBTAIN_FILESIZE << e.what() << std::endl;
    }
    return size;
}

/**
 * @brief Checks if a log file needs rotation based on its size
 * @param path The filesystem path of the log file to check
 * @return true if file size exceeds MAX_ERROR_LOG_SIZE
 * @return false if file size is within limits or couldn't be checked
 */
bool FSHelper::needLogRotation(const std::string& path)
{
    return FSHelper::fileSize(path) > MAX_ERROR_LOG_SIZE;
}

/**
 * @brief Performs log rotation by deleting the specified log file
 * @param path The filesystem path of the log file to rotate
 * @return true if log file was successfully deleted
 * @return false if deletion failed (error will be printed to stderr)
 */
bool FSHelper::rotateLog(const std::string& path)
{
    std::string errMsg;
    if(!FSHelper::deleteFile(path, errMsg))
    {
        std::cerr << ERR_MSG_UNABLE_DELETE_ERRLOG << errMsg << std::endl;
        return false;
    }
    return true;
}

