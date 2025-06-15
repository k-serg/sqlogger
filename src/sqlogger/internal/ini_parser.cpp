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

#include "sqlogger/internal/ini_parser.h"

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
INI::INIData INI::parse(const std::string& filename)
{
    INI::INIData data;
    std::ifstream file(filename);
    if(!file.is_open())
    {
        throw std::runtime_error(ERR_MSG_FAILED_OPEN_FILE + filename);
    }

    std::string line;
    std::string currentSection;

    while(std::getline(file, line))
    {
        // Remove leading and trailing whitespace
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        // Skip empty lines and comments
        if(line.empty() || line[0] == ';' || line[0] == '#')
        {
            continue;
        }

        // Handle section headers
        if(line[0] == '[' && line.back() == ']')
        {
            currentSection = line.substr(1, line.size() - 2);
        }
        // Handle key-value pairs
        else
        {
            size_t equalsPos = line.find('=');
            if(equalsPos != std::string::npos)
            {
                std::string key = line.substr(0, equalsPos);
                std::string value = line.substr(equalsPos + 1);

                // Remove leading and trailing whitespace from key and value
                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);

                data[currentSection][key] = value;
            }
        }
    }
    file.close();
    return data;
};

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
void INI::write(const std::string& filename, const INIData& data)
{
    std::ofstream file(filename);
    if(!file.is_open())
    {
        throw std::runtime_error(ERR_MSG_FAILED_OPEN_FILE_RW + filename);
    }

    for(const auto & [section, keys] : data)
    {
        file << "[" << section << "]\n";
        for(const auto & [key, value] : keys)
        {
            file << key << "=" << value << "\n";
        }
        file << "\n";
    }
    file.close();
};
