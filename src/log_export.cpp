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

#include "log_export.h"

/**
 * @brief Exports log entries to a text file.
 * @param filePath The path to the output file.
 * @param entryList The list of log entries to export.
 * @param delimiter The delimiter to use between fields.
 * @param name Whether to include field names in the output.
 */
void LogExport::exportToTXT(const std::string& filePath, const LogEntryList& entryList, const std::string& delimiter, bool name)
{
    std::ofstream outFile(filePath);
    if(!outFile.is_open())
    {
        throw std::runtime_error("Failed to open file: " + filePath);
    }
    for(const auto & entry : entryList)
    {
        outFile << entry.print(delimiter, name) << std::endl;
    }
}

/**
 * @brief Exports log entries to a CSV file.
 * @param filePath The path to the output file.
 * @param entryList The list of log entries to export.
 * @param delimiter The delimiter to use between fields.
 */
void LogExport::exportToCSV(const std::string& filePath, const LogEntryList& entryList, const std::string& delimiter)
{
    std::ofstream outFile(filePath);
    if(!outFile.is_open())
    {
        throw std::runtime_error("Failed to open file: " + filePath);
    }
    outFile << "ID" << delimiter
            << "Timestamp" << delimiter
            << "Level" << delimiter
            << "Message" << delimiter
            << "Function" << delimiter
            << "File" << delimiter
            << "Line" << delimiter
            << "Thread ID" << std::endl;
    for(const auto & entry : entryList)
    {
        outFile << entry.id << delimiter
                << entry.timestamp << delimiter
                << entry.level << delimiter
                << "\"" << entry.message << "\"" << delimiter
                << entry.function << delimiter
                << entry.file << delimiter
                << entry.line << delimiter
                << entry.threadId << std::endl;
    }
}

/**
 * @brief Exports log entries to an XML file.
 * @param filePath The path to the output file.
 * @param entryList The list of log entries to export.
 */
void LogExport::exportToXML(const std::string& filePath, const LogEntryList& entryList)
{
    std::ofstream outFile(filePath);
    if(!outFile.is_open())
    {
        throw std::runtime_error("Failed to open file: " + filePath);
    }
    outFile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
    outFile << "<LogEntries>" << std::endl;
    for(const auto & entry : entryList)
    {
        outFile << "  <LogEntry>" << std::endl;
        outFile << "    <ID>" << entry.id << "</ID>" << std::endl;
        outFile << "    <Timestamp>" << entry.timestamp << "</Timestamp>" << std::endl;
        outFile << "    <Level>" << entry.level << "</Level>" << std::endl;
        outFile << "    <Message>" << entry.message << "</Message>" << std::endl;
        outFile << "    <Function>" << entry.function << "</Function>" << std::endl;
        outFile << "    <File>" << entry.file << "</File>" << std::endl;
        outFile << "    <Line>" << entry.line << "</Line>" << std::endl;
        outFile << "    <ThreadID>" << entry.threadId << "</ThreadID>" << std::endl;
        outFile << "  </LogEntry>" << std::endl;
    }
    outFile << "</LogEntries>" << std::endl;
}

/**
 * @brief Exports log entries to a JSON file.
 * @param filePath The path to the output file.
 * @param entryList The list of log entries to export.
 */
void LogExport::exportToJSON(const std::string& filePath, const LogEntryList& entryList)
{
    std::ofstream outFile(filePath);
    if(!outFile.is_open())
    {
        throw std::runtime_error("Failed to open file: " + filePath);
    }
    outFile << "[" << std::endl;
    for(size_t i = 0; i < entryList.size(); ++i)
    {
        const auto& entry = entryList[i];
        outFile << "  {" << std::endl
                << "    \"ID\": " << entry.id << "," << std::endl
                << "    \"Timestamp\": \"" << escapeJsonString(entry.timestamp) << "\"," << std::endl
                << "    \"Level\": \"" << escapeJsonString(entry.level) << "\"," << std::endl
                << "    \"Message\": \"" << escapeJsonString(entry.message) << "\"," << std::endl
                << "    \"Function\": \"" << escapeJsonString(entry.function) << "\"," << std::endl
                << "    \"File\": \"" << escapeJsonString(entry.file) << "\"," << std::endl
                << "    \"Line\": " << entry.line << "," << std::endl
                << "    \"ThreadID\": \"" << escapeJsonString(entry.threadId) << "\"" << std::endl
                << "  }" << (i < entryList.size() - 1 ? "," : "") << std::endl;
    }
    outFile << "]" << std::endl;
}

/**
 * @brief Escapes special characters in a JSON string.
 * @param str The string to escape.
 * @return The escaped string.
 */
std::string LogExport::escapeJsonString(const std::string& str)
{
    std::string result;
    for(char ch : str)
    {
        switch(ch)
        {
            case '\\':
                result += "\\\\";
                break;
            case '\"':
                result += "\\\"";
                break;
            case '\b':
                result += "\\b";
                break;
            case '\f':
                result += "\\f";
                break;
            case '\n':
                result += "\\n";
                break;
            case '\r':
                result += "\\r";
                break;
            case '\t':
                result += "\\t";
                break;
            default:
                result += ch;
                break;
        }
    }
    return result;
}

/**
* @brief Exports log entries to a specified format file.
* @param filePath The path to the output file.
* @param format Format of the output file.
* @param entryList The list of log entries to export.
* @param delimiter The delimiter to use between fields.
* @param name Whether to include field names in the output.
*/
void LogExport::exportTo(const std::string& filePath, const Format& format, const LogEntryList& entryList, const std::string& delimiter, bool name)
{
    switch(format)
    {
        case Format::TXT:
            exportToTXT(filePath, entryList, delimiter, name);
            break;
        case Format::CSV:
            exportToCSV(filePath, entryList, delimiter);
            break;
        case Format::XML:
            exportToXML(filePath, entryList);
            break;
        case Format::JSON:
            exportToJSON(filePath, entryList);
            break;
        default:
            throw std::runtime_error("Unknown export format");
            break;
    }
}