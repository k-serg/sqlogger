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

#include "fs_helper.h"

/**
 * @brief Create directories from path.
 * @param path Path to the file/directory.
 * @param errMsg Error message.
 * If path contains filename it will be removed.
 * @return bool True, if path created or already exists.
 * False otherwise.
*/

bool FSHelper::CreateDir(const std::string& path, std::string& errMsg)
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
