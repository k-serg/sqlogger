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

#ifndef LOG_CONFIG_H
#define LOG_CONFIG_H

#include <optional>

// Defaults
#define LOG_NUM_THREADS 4
#define LOG_SYNC_MODE 1
#define LOG_ONLY_FILE_NAMES 0

namespace LogConfig
{
    struct Config
    {
        std::optional<bool> syncMode = LOG_SYNC_MODE;
        std::optional<size_t> numThreads = LOG_NUM_THREADS;
        std::optional<bool> onlyFileNames = LOG_ONLY_FILE_NAMES;
    };
}

#endif // !LOG_CONFIG_H
