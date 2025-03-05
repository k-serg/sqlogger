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

#ifndef LOG_EXPORT_H
#define LOG_EXPORT_H

#include <ostream>
#include <string>
#include <stdexcept>
#include "log_entry.h"
#include "fs_helper.h"

namespace LogExport
{
    enum class Format
    {
        TXT,
        CSV,
        XML,
        JSON
    };

    /**
    * @brief Exports log entries to a specified format file.
    * @param filePath The path to the output file.
    * @param format Format of the output file.
    * @param entryList The list of log entries to export.
    * @param delimiter The delimiter to use between fields.
    * @param name Whether to include field names in the output.
    */
    void exportTo(const std::string& filePath, const Format& format, const LogEntryList& entryList, const std::string& delimiter = ENTRY_DELIMITER, bool name = true);

    /**
    * @brief Exports log entries to a text file.
    * @param filePath The path to the output file.
    * @param entryList The list of log entries to export.
    * @param delimiter The delimiter to use between fields.
    * @param name Whether to include field names in the output.
    */
    void exportToTXT(const std::string& filePath, const LogEntryList& entryList, const std::string& delimiter = ENTRY_DELIMITER, bool name = true);

    /**
     * @brief Exports log entries to a CSV file.
     * @param filePath The path to the output file.
     * @param entryList The list of log entries to export.
     * @param delimiter The delimiter to use between fields.
     */
    void exportToCSV(const std::string& filePath, const LogEntryList& entryList, const std::string& delimiter = ENTRY_DELIMITER);

    /**
     * @brief Exports log entries to an XML file.
     * @param filePath The path to the output file.
     * @param entryList The list of log entries to export.
     */
    void exportToXML(const std::string& filePath, const LogEntryList& entryList);

    /**
     * @brief Exports log entries to a JSON file.
     * @param filePath The path to the output file.
     * @param entryList The list of log entries to export.
     */
    void exportToJSON(const std::string& filePath, const LogEntryList& entryList);

    /**
     * @brief Escapes special characters in a JSON string.
     * @param str The string to escape.
     * @return The escaped string.
     */
    std::string escapeJsonString(const std::string& str);
};

#endif // !LOG_EXPORT_H
