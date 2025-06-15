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

#include "sqlogger/internal/log_serializer.h"

std::string JsonParser::getString(const std::string& json, const std::string& key)
{
    size_t pos = json.find("\"" + key + "\":");
    if(pos == std::string::npos) throw std::runtime_error("Key '" + key + "' not found");

    size_t start = json.find('"', pos + key.length() + 3) + 1;
    size_t end = json.find('"', start);
    return json.substr(start, end - start);
}

int JsonParser::getInt(const std::string& json, const std::string& key)
{
    size_t pos = json.find("\"" + key + "\":");
    if(pos == std::string::npos) throw std::runtime_error("Key '" + key + "' not found");

    size_t start = json.find_first_of("0123456789-", pos + key.length() + 3);
    size_t end = json.find_first_not_of("0123456789", start);
    return std::stoi(json.substr(start, end - start));
}

/**
 * @brief Escapes special characters in a JSON string.
 * @param str The string to escape.
 * @return The escaped string.
 */
std::string JsonParser::escapeJsonString(const std::string& str)
{
    std::ostringstream oss;
    for(char c : str)
    {
        switch(c)
        {
            case '"':
                oss << "\\\"";
                break;
            case '\\':
                oss << "\\\\";
                break;
            case '\b':
                oss << "\\b";
                break;
            case '\f':
                oss << "\\f";
                break;
            case '\n':
                oss << "\\n";
                break;
            case '\r':
                oss << "\\r";
                break;
            case '\t':
                oss << "\\t";
                break;
            default:
                if(static_cast<unsigned char>(c) <= 0x1F)
                {
                    oss << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                        << static_cast<int>(c);
                }
                else
                {
                    oss << c;
                }
        }
    }
    return oss.str();
}

std::string LogSerializer::Json::serializeLog(const LogEntry& entry)
{
#ifdef SQLG_USE_EXTERNAL_JSON_PARSER

    nlohmann::json json;

    json[EXP_FIELD_ID] = entry.id;
    json[EXP_FIELD_TIMESTAMP] = entry.timestamp;
    json[EXP_FIELD_LEVEL] = entry.level;
    json[EXP_FIELD_MESSAGE] = entry.message;
    json[EXP_FIELD_FUNCTION] = entry.function;
    json[EXP_FIELD_FILE] = entry.file;
    json[EXP_FIELD_LINE] = entry.line;
    json[EXP_FIELD_THREAD_ID] = entry.threadId;
    json[EXP_FIELD_LEVEL] = entry.level;

#ifdef SQLG_USE_SOURCE_INFO
    nlohmann::json source;
    source[EXP_FIELD_SOURCE_ID] = entry.sourceId;
    source[EXP_FIELD_SOURCE_UUID] = entry.sourceUuid;
    source[EXP_FIELD_SOURCE_NAME] = entry.sourceName;
    json[EXP_FIELD_SOURCE] = source;
#endif
    return json.dump(2);

#else
    std::ostringstream oss;
    oss << "  {" << std::endl
        << "    \"" << EXP_FIELD_ID << "\": " << entry.id << "," << std::endl
        << "    \"" << EXP_FIELD_TIMESTAMP << "\": \"" << JsonParser::escapeJsonString(entry.timestamp) << "\"," << std::endl
        << "    \"" << EXP_FIELD_LEVEL << "\": \"" << JsonParser::escapeJsonString(entry.level) << "\"," << std::endl
        << "    \"" << EXP_FIELD_MESSAGE << "\": \"" << JsonParser::escapeJsonString(entry.message) << "\"," << std::endl
        << "    \"" << EXP_FIELD_FUNCTION << "\": \"" << JsonParser::escapeJsonString(entry.function) << "\"," << std::endl
        << "    \"" << EXP_FIELD_FILE << "\": \"" << JsonParser::escapeJsonString(entry.file) << "\"," << std::endl
        << "    \"" << EXP_FIELD_LINE << "\": " << entry.line << "," << std::endl
        << "    \"" << EXP_FIELD_THREAD_ID << "\": \"" << JsonParser::escapeJsonString(entry.threadId) << "\"";
#ifdef SQLG_USE_SOURCE_INFO
    oss << "," << std::endl
        << "    \"" << EXP_FIELD_SOURCE << "\": " << std::endl
        << "    {" << std::endl
        << "      \"" << EXP_FIELD_SOURCE_ID << "\": \"" << entry.sourceId << "\"," << std::endl
        << "      \"" << EXP_FIELD_SOURCE_UUID << "\": \"" << JsonParser::escapeJsonString(entry.sourceUuid) << "\"," << std::endl
        << "      \"" << EXP_FIELD_SOURCE_NAME << "\": \"" << JsonParser::escapeJsonString(entry.sourceName) << "\"" << std::endl
        << "    }";
#endif
    oss << std::endl << "  }";
    return oss.str();
#endif
}

std::string LogSerializer::Json::serializeLogs(const LogEntryList& entries)
{
    std::ostringstream oss;
    oss << "[" << std::endl;
    for(size_t i = 0; i < entries.size(); ++i)
    {
        const auto& entry = entries[i];
        oss << LogSerializer::Json::serializeLog(entry);
        oss << (i < entries.size() - 1 ? "," : "") << std::endl;
    }
    oss << "]" << std::endl;
    return oss.str();
}

std::string LogSerializer::Json::serializeFilter(const Filter& filter)
{
#ifdef SQLG_USE_EXTERNAL_JSON_PARSER
    nlohmann::json json;

    json[EXP_FILTER_FIELD] = filter.field;
    json[EXP_FILTER_OP] = filter.op;
    json[EXP_FILTER_VALUE] = filter.value;

    return json.dump(2);
#else
    std::ostringstream oss;
    oss << "  {" << std::endl
        << "    \"" << EXP_FILTER_FIELD << "\": " << filter.field << "\"," << std::endl
        << "    \"" << EXP_FILTER_OP << "\": \"" << filter.op << "\"," << std::endl
        << "    \"" << EXP_FILTER_VALUE << "\": \"" << filter.value << "\"," << std::endl
        << std::endl << "  }" << std::endl;
    return oss.str();
#endif
}

std::string LogSerializer::Json::serializeFilters(const std::vector<Filter> & filters)
{
    std::ostringstream oss;
    for(size_t i = 0; i < filters.size(); ++i)
    {
        const auto& filter = filters[i];
        oss << LogSerializer::Json::serializeFilter(filter)
            << (i < filters.size() - 1 ? "," : "") << std::endl;
    }
    return oss.str();
}

LogEntry LogSerializer::Json::parseLog(const std::string& jsonString)
{
    LogEntry entry;

#ifdef SQLG_USE_EXTERNAL_JSON_PARSER
    try
    {
        auto json = nlohmann::json::parse(jsonString);

        entry.id = json.at(EXP_FIELD_ID).get<int>();
        entry.timestamp = json.at(EXP_FIELD_TIMESTAMP).get<std::string>();
        entry.level = json.at(EXP_FIELD_LEVEL).get<std::string>();
        entry.message = json.at(EXP_FIELD_MESSAGE).get<std::string>();
        entry.function = json.at(EXP_FIELD_FUNCTION).get<std::string>();
        entry.file = json.at(EXP_FIELD_FILE).get<std::string>();
        entry.line = json.at(EXP_FIELD_LINE).get<int>();
        entry.threadId = json.at(EXP_FIELD_THREAD_ID).get<std::string>();

#ifdef SQLG_USE_SOURCE_INFO
        if(json.contains(EXP_FIELD_SOURCE))
        {
            const auto& source = json.at(EXP_FIELD_SOURCE);
            entry.sourceId = source.at(EXP_FIELD_SOURCE_ID).get<int>();
            entry.sourceUuid = source.at(EXP_FIELD_SOURCE_UUID).get<std::string>();
            entry.sourceName = source.at(EXP_FIELD_SOURCE_NAME).get<std::string>();
        }
#endif
    }
    catch(const std::exception& e)
    {
        throw std::runtime_error("Failed to parse LogEntry: " + std::string(e.what()));
    }

#else
    try
    {
        entry.id = JsonParser::getInt(jsonString, EXP_FIELD_ID);
        entry.timestamp = JsonParser::getString(jsonString, EXP_FIELD_TIMESTAMP);
        entry.level = JsonParser::getString(jsonString, EXP_FIELD_LEVEL);
        entry.message = JsonParser::getString(jsonString, EXP_FIELD_MESSAGE);
        entry.function = JsonParser::getString(jsonString, EXP_FIELD_FUNCTION);
        entry.file = JsonParser::getString(jsonString, EXP_FIELD_FILE);
        entry.line = JsonParser::getInt(jsonString, EXP_FIELD_LINE);
        entry.threadId = JsonParser::getString(jsonString, EXP_FIELD_THREAD_ID);

#ifdef SQLG_USE_SOURCE_INFO
        size_t source_start = jsonString.find("\"" + std::string(EXP_FIELD_SOURCE) + "\":{");
        if(source_start != std::string::npos)
        {
            entry.sourceId = JsonParser::getInt(jsonString.substr(source_start), EXP_FIELD_SOURCE_ID);
            entry.sourceUuid = JsonParser::getString(jsonString.substr(source_start), EXP_FIELD_SOURCE_UUID);
            entry.sourceName = JsonParser::getString(jsonString.substr(source_start), EXP_FIELD_SOURCE_NAME);
        }
#endif
    }
    catch(const std::exception& e)
    {
        throw std::runtime_error("Failed to parse LogEntry: " + std::string(e.what()));
    }
#endif

    return entry;
}

LogEntryList LogSerializer::Json::parseLogs(const std::string& jsonArray)
{
    LogEntryList logs;

#ifdef SQLG_USE_EXTERNAL_JSON_PARSER
    try
    {
        nlohmann::json json = nlohmann::json::parse(jsonArray);
        if(json.is_array())
        {
            for(const auto & item : json)
            {
                logs.push_back(LogSerializer::Json::parseLog(item.dump()));
            }
        }
    }
    catch(const std::exception& e)
    {
        throw std::runtime_error("Failed to parse logs array: " + std::string(e.what()));
    }

#else
    size_t start_pos = jsonArray.find('[');
    size_t end_pos = jsonArray.rfind(']');

    if(start_pos == std::string::npos || end_pos == std::string::npos)
    {
        throw std::runtime_error("Invalid JSON array format");
    }

    std::string content = jsonArray.substr(start_pos + 1, end_pos - start_pos - 1);
    size_t brace_level = 0;
    size_t item_start = 0;

    for(size_t i = 0; i < content.size(); ++i)
    {
        if(content[i] == '{')
        {
            if(brace_level == 0) item_start = i;
            brace_level++;
        }
        else if(content[i] == '}')
        {
            brace_level--;
            if(brace_level == 0)
            {
                std::string item = content.substr(item_start, i - item_start + 1);
                logs.push_back(parseLog(item));
            }
        }
    }
#endif

    return logs;
}

std::vector<Filter> LogSerializer::Json::parseFilters(const std::string& jsonString)
{
    std::vector<Filter> filters;

#ifdef  SQLG_USE_EXTERNAL_JSON_PARSER
    try
    {
        auto json = nlohmann::json::parse(jsonString);

        if(json.is_array())
        {
            for(const auto & filterJson : json)
            {
                Filter filter;
                filter.field = filterJson.at(EXP_FILTER_FIELD).get<std::string>();
                filter.op = filterJson.at(EXP_FILTER_OP).get<std::string>();
                filter.value = filterJson.at(EXP_FILTER_VALUE).get<std::string>();
                filter.type = Filter::fieldToType(filter.field);
                filters.push_back(filter);
            }
        }
        else if(json.is_object())
        {
            Filter filter;
            filter.field = json.at(EXP_FILTER_FIELD).get<std::string>();
            filter.op = json.at(EXP_FILTER_OP).get<std::string>();
            filter.value = json.at(EXP_FILTER_VALUE).get<std::string>();
            filter.type = Filter::fieldToType(filter.field);
            filters.push_back(filter);
        }
        else
        {
            throw std::runtime_error("Invalid filters format - expected array or object");
        }
    }
    catch(const std::exception& e)
    {
        throw std::runtime_error("Invalid filter format: " + std::string(e.what()));
    }

#else
    size_t pos = 0;
    while((pos = jsonString.find('{', pos)) != std::string::npos)
    {
        size_t end = jsonString.find('}', pos);
        if(end == std::string::npos) break;

        std::string filterStr = jsonString.substr(pos, end - pos + 1);

        Filter filter;
        try
        {
            filter.field = JsonParser::getString(filterStr, EXP_FILTER_FIELD);
            filter.op = JsonParser::getString(filterStr, EXP_FILTER_OP);
            filter.value = JsonParser::getString(filterStr, EXP_FILTER_VALUE);
            filter.type = Filter::fieldToType(filter.field);
            filters.push_back(filter);
        }
        catch(const std::exception& e)
        {
            throw std::runtime_error("Invalid filter format: " + std::string(e.what()));
        }

        pos = end + 1;
    }
#endif

    return filters;
}
