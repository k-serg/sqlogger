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
#include <string>
#include "sqlogger_config.h"
#include "log_entry.h"

// Defaults
#define LOG_NUM_THREADS 4 ///< Default number of threads for asynchronous logging.
#define LOG_SYNC_MODE 1 ///< Default synchronization mode (true for synchronous logging).
#define LOG_ONLY_FILE_NAMES 0 ///< Default whether to log only filenames (without full paths).
constexpr LogLevel LOG_MIN_LOG_LEVEL = LogLevel::Trace; ///< Default minimum log level for messages to be logged.

#define LOG_INI_SECTION "Logger"
#define LOG_INI_KEY_SYNC_MODE "SyncMode"
#define LOG_INI_KEY_NUM_THREADS "NumThreads"
#define LOG_INI_KEY_ONLY_FILE_NAMES "OnlyFileNames"
#define LOG_INI_KEY_MIN_LOG_LEVEL "MinLogLevel"

constexpr char* LOG_INI_FILENAME = SQLOGGER_PROJECT_NAME ".ini";

namespace LogConfig
{
    /**
     * @brief Configuration settings for the logger.
     */
    struct Config
    {
        std::optional<bool> syncMode = LOG_SYNC_MODE; ///< Synchronization mode (true for synchronous logging).
        std::optional<size_t> numThreads = LOG_NUM_THREADS; ///< Number of threads for asynchronous logging.
        std::optional<bool> onlyFileNames = LOG_ONLY_FILE_NAMES; ///< Whether to log only filenames (without full paths).
        std::optional<LogLevel> minLogLevel = LOG_MIN_LOG_LEVEL; ///< Minimum log level for messages to be logged.

        /**
        * @brief Loads configuration from an INI file.
        * @param filename The path to the INI file.
        * @return A Config object containing the loaded settings.
        * @throws std::runtime_error If the file cannot be parsed.
        */
        static Config loadFromINI(const std::string& filename = LOG_INI_FILENAME);

        /**
         * @brief Saves configuration to an INI file.
         * @param config The Config object to save.
         * @param filename The path to the INI file.
         * @throws std::runtime_error If the file cannot be written.
         */
        static void saveToINI(const Config& config, const std::string& filename = LOG_INI_FILENAME);
    };
};

#endif // !LOG_CONFIG_H
