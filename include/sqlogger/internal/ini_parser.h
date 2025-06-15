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

#ifndef INI_PARSER_H
#define INI_PARSER_H

#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "log_strings.h"

/**
 * @namespace INI
 * @brief Provides functionality for reading and writing INI configuration files.
 *
 * @details This namespace contains utilities for parsing INI files into a nested map structure
 * and writing data back to INI format. It supports standard INI file format with sections,
 * key-value pairs, and basic type conversion.
 *
 * The INI format structure follows:
 * - Sections marked with [section_name]
 * - Key-value pairs in format key=value
 * - Comments starting with ; or #
 * - Empty lines are ignored
 *
 * @example
 * // Example INI file content:
 * // [Database]
 * // host=localhost
 * // port=3306
 * //
 * // [Logging]
 * // level=DEBUG
 */
namespace INI
{
    // Alias for a nested map representing INI data
    using INIData = std::map<std::string, std::map<std::string, std::string>>;

    /**
     * @brief Parses an INI file and returns its content as a nested map.
     *
     * @param filename The path to the INI file.
     * @return INIData A map where the key is the section name, and the value is a map of key-value pairs.
     * @throws std::runtime_error If the file cannot be opened or contains malformed data.
     * @note Leading/trailing whitespace is automatically trimmed from section names, keys and values
     * @note Empty sections are preserved in the returned structure
     * @note Duplicate keys in the same section will overwrite previous values
     */
    INIData parse(const std::string& filename);

    /**
     * @brief Writes data to an INI file.
     *
     * @param filename The path to the INI file.
     * @param data INIData A map where the key is the section name, and the value is a map of key-value pairs.
     * @throws std::runtime_error If the file cannot be opened for writing.
     * @note Sections will be written in alphabetical order
     * @note Keys within each section will be written in alphabetical order
     * @note Empty sections will be written as [section] with no key-value pairs
     * @note Special characters (=, ;, #) in values will be properly escaped
     */
    void write(const std::string& filename, const INIData& data);
};

#endif // INI_PARSER_H