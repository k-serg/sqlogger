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
#include "sqlogger/log_entry.h"
#include "sqlogger/internal/fs_helper.h"
#include "sqlogger/internal/log_serializer.h"

/**
 * @namespace LogExport
 * @brief Provides functionality for exporting log entries to various file formats
 *
 * @details This namespace contains utilities for converting log entries into different
 * output formats and writing them to files. It supports multiple standardized formats
 * suitable for different use cases including analysis, reporting, and data interchange.
 *
 * Supported formats:
 * @see Format for supported output formats
 *
 * @see LogEntry for the structure of log entries
 * @see ENTRY_DELIMITER for default delimiter values
 */
namespace LogExport
{

    /**
    * @enum Format
    * @brief Supported output formats for log export functionality
    *
    * @details Defines the various standardized formats available for exporting log entries.
    * Each format is optimized for different use cases and interoperability requirements.
    *
    * The available formats are:
    * @value TXT Plain text format - Human-readable, simple structure
    * @value CSV Comma-Separated Values - Spreadsheet/analysis friendly
    * @value XML eXtensible Markup Language - Structured, self-describing
    * @value JSON JavaScript Object Notation - Web/API friendly
    * @value YAML YAML Ain't Markup Language - Human-readable configuration
    *
    * @note Default format is TXT when not explicitly specified
    * @see exportTo() for the main export function using this enum
    * @see LogEntry for the data structure being exported
    */
    enum class Format
    {
        TXT,
        CSV,
        XML,
        JSON,
        YAML
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
    * @brief Exports log entries to a YAML file.
    * @param filePath The path to the output file.
    * @param entryList The list of log entries to export.
    */
    void exportToYAML(const std::string& filePath, const LogEntryList& entryList);

    /**
    * @brief Escapes special characters in a YAML string.
    * @param str The string to escape.
    * @return The escaped string.
    */
    std::string escapeYamlString(const std::string& str);
};

#endif // !LOG_EXPORT_H
