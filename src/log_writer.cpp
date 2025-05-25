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

#include "log_writer.h"

/**
 * @brief Writes a log entry to the database.
 * @param entry The log entry to write.
 * @return True if the log entry was written successfully, false otherwise.
 */
bool LogWriter::writeLog(const LogEntry& entry)
{
    std::vector<std::pair<std::string, std::string>> values =
    {
#ifdef USE_SOURCE_INFO
        {FIELD_LOG_SOURCES_ID, std::to_string(entry.sourceId)},
#endif
        {FIELD_LOG_TIMESTAMP, entry.timestamp},
        {FIELD_LOG_LEVEL, entry.level},
        {FIELD_LOG_MESSAGE, entry.message},
        {FIELD_LOG_FUNCTION, entry.function},
        {FIELD_LOG_FILE, entry.file},
        {FIELD_LOG_LINE, std::to_string(entry.line)},
        {FIELD_LOG_THREAD_ID, entry.threadId}
    };

    std::string query = QueryBuilder::buildInsert(
                            database.getDatabaseType(),
                            LOG_TABLE_NAME,
                            values
                        );

    std::vector<std::string> params;
    for(const auto & [_, value] : values)
    {
        params.push_back(value);
    }

    return database.execute(query, params);
}

/**
 * @brief Executes a batch insert of log entries into the database.
 * Constructs and executes a parameterized batch INSERT query optimized for the current database type.
 * @param entries List of log entries to insert. Each entry must contain all required fields.
 * @return bool True if the batch insert succeeded, false otherwise.
 * @throws std::runtime_error If database execution fails (handled internally).
 */
bool LogWriter::writeLogBatch(const LogEntryList& entries)
{
    if(entries.empty()) return true;

    const std::vector<std::string> fields =
    {
#ifdef USE_SOURCE_INFO
        FIELD_LOG_SOURCES_ID,
#endif
        FIELD_LOG_TIMESTAMP,
        FIELD_LOG_LEVEL,
        FIELD_LOG_MESSAGE,
        FIELD_LOG_FUNCTION,
        FIELD_LOG_FILE,
        FIELD_LOG_LINE,
        FIELD_LOG_THREAD_ID
    };

    std::string query = QueryBuilder::buildBatchInsert(
                            LOG_TABLE_NAME,
                            fields,
                            entries.size(),
                            database.getDatabaseType()
                        );

    // SQL
    std::vector<std::string> params;
    if(database.getDatabaseType() != DataBaseType::MongoDB)
    {
        params.reserve(entries.size() * fields.size());
        for(const auto & entry : entries)
        {
#ifdef USE_SOURCE_INFO
            params.push_back(std::to_string(entry.sourceId));
#endif
            params.push_back(entry.timestamp);
            params.push_back(entry.level);
            params.push_back(entry.message);
            params.push_back(entry.function);
            params.push_back(entry.file);
            params.push_back(std::to_string(entry.line));
            params.push_back(entry.threadId);
        }
    }

    return database.execute(query, params);
}

/**
 * @brief Clears all log entries from the database.
 */
void LogWriter::clearLogs()
{
    std::string query = QueryBuilder::buildDelete(
                            database.getDatabaseType(),
                            LOG_TABLE_NAME,
                            {} // No filters
                        );

    database.execute(query);
}

#ifdef USE_SOURCE_INFO
/**
* @brief Clears all source entries from the database.
*/
void LogWriter::clearSources()
{
    std::string query = QueryBuilder::buildDelete(
                            database.getDatabaseType(),
                            SOURCES_TABLE_NAME,
                            {} // No filters
                        );

    database.execute(query);
}
#endif

/**
 * @brief Creates the log table in the database if it does not exist.
 */
void LogWriter::createLogsTable()
{
    std::string checkQueryLogs = QueryBuilder::buildTableExistsQuery(
                                     database.getDatabaseType(),
                                     LOG_TABLE_NAME
                                 );

#ifdef USE_SOURCE_INFO
    std::string checkQuerySources = QueryBuilder::buildTableExistsQuery(
                                        database.getDatabaseType(),
                                        SOURCES_TABLE_NAME
                                    );
#endif

    if(!checkQueryLogs.empty() && !database.query(checkQueryLogs).empty()
#ifdef USE_SOURCE_INFO
            && !database.query(checkQuerySources).empty()
#endif
      )
        return; // Skip if exists

    auto logTable = DatabaseSchema::createTableBuilder(LOG_TABLE_NAME)
                    .addStandardField<FieldType::Int64>(FIELD_LOG_ID, true, false, true)  // PRIMARY AUTOINCREMENT KEY
#ifdef USE_SOURCE_INFO
                    .addStandardField<FieldType::Int64>(FIELD_LOG_SOURCES_ID, false, false, false)
                    .addForeignKey(FIELD_LOG_SOURCES_ID, SOURCES_TABLE_NAME, FIELD_SOURCES_ID)
#endif
                    .addStandardField<FieldType::DateTime>(FIELD_LOG_TIMESTAMP, false, false)
                    .addStandardField<FieldType::String>(FIELD_LOG_LEVEL, false, false)
                    .addStandardField<FieldType::String>(FIELD_LOG_MESSAGE, false, false)
                    .addStandardField<FieldType::String>(FIELD_LOG_FUNCTION, false, false)
                    .addStandardField<FieldType::String>(FIELD_LOG_FILE, false, false)
                    .addStandardField<FieldType::Int32>(FIELD_LOG_LINE, false, false)
                    .addStandardField<FieldType::String>(FIELD_LOG_THREAD_ID, false, false)
                    .build();

    std::string query = QueryBuilder::buildCreateTable(
                            logTable,
                            database.getDatabaseType()
                        );

    if(!query.empty())
    {
        database.execute(query);
    }
}

/**
 * @brief Creates indexes on the log table for faster queries.
 */
void LogWriter::createIndexes()
{
    const std::string idxPrefix = "idx_";

    std::vector<std::string> logIndexes =
    {
        FIELD_LOG_TIMESTAMP,
        FIELD_LOG_LEVEL,
        FIELD_LOG_FILE,
        FIELD_LOG_THREAD_ID,
        FIELD_LOG_FUNCTION
    };

    for(const auto & field : logIndexes)
    {
        std::string checkQuery = QueryBuilder::buildIndexExistsQuery(
                                     database.getDatabaseType(),
                                     idxPrefix + field
                                 );

        if(!checkQuery.empty() && !database.query(checkQuery).empty())
            continue; // Skip if exists

        std::string query = QueryBuilder::buildCreateIndex(
                                database.getDatabaseType(),
                                LOG_TABLE_NAME,
                                idxPrefix + field,
        { field }
                            );

        if(!query.empty())
        {
            database.execute(query);
        }
    }

#ifdef USE_SOURCE_INFO
    std::vector<std::string> sourceIndexes =
    {
        FIELD_SOURCES_UUID,
        FIELD_SOURCES_NAME
    };

    for(const auto & field : sourceIndexes)
    {
        std::string checkQuery = QueryBuilder::buildIndexExistsQuery(
                                     database.getDatabaseType(),
                                     idxPrefix + field
                                 );

        if(!database.query(checkQuery).empty())
            continue; // Skip if exists

        std::string query = QueryBuilder::buildCreateIndex(
                                database.getDatabaseType(),
                                SOURCES_TABLE_NAME,
                                idxPrefix + field,
        { field }
                            );

        if(!query.empty())
        {
            database.execute(query);
        }
    }
#endif
}

#ifdef USE_SOURCE_INFO
/**
 * @brief Creates the sources table in the database if it does not exist.
 * This method creates a table for storing source information (e.g., UUID, name) in the database.
 * If the table already exists, it will not be recreated.
 */
void LogWriter::createSourcesTable()
{
    std::string checkQuerySources = QueryBuilder::buildTableExistsQuery(
                                        database.getDatabaseType(),
                                        SOURCES_TABLE_NAME
                                    );

    if(!database.query(checkQuerySources).empty())
        return; // Skip if exists

#ifdef USE_SOURCE_INFO
    auto sourcesTable = DatabaseSchema::createTableBuilder(SOURCES_TABLE_NAME)
                        .addStandardField<FieldType::Int64>(FIELD_SOURCES_ID, true, false, true) // PRIMARY AUTOINCREMENT KEY
                        .addStandardField<FieldType::UUID>(FIELD_SOURCES_UUID, false, false, false, true) // UNIQUE KEY
                        .addStandardField<FieldType::String>(FIELD_SOURCES_NAME, false, false, false)
                        .build();
#endif

    std::string query = QueryBuilder::buildCreateTable(
                            sourcesTable,
                            database.getDatabaseType()
                        );

    if(!query.empty())
    {
        database.execute(query);
    }
}
#endif

#ifdef USE_SOURCE_INFO
/**
 * @brief Adds a new source to the database.
 * @param name The name of the source.
 * @param uuid The UUID of the source.
 * @return The ID of the newly added source, or SOURCE_NOT_FOUND if the operation failed.
 */
int LogWriter::addSource(const std::string& name, const std::string& uuid)
{
    std::vector<std::pair<std::string, std::string>> values =
    {
        {FIELD_SOURCES_UUID, uuid.empty() ? LogHelper::generateUUID() : uuid},
        {FIELD_SOURCES_NAME, name}
    };

    // Build INSERT
    std::string insertQuery = QueryBuilder::buildInsert(
                                  database.getDatabaseType(),
                                  SOURCES_TABLE_NAME,
                                  values
                              );

    if(!database.execute(insertQuery, { uuid, name }))
    {
        return SOURCE_NOT_FOUND;
    }

    // Get ID through SELECT
    std::vector<Filter> filters =
    {
        {Filter::Type::Unknown, FIELD_SOURCES_UUID, "=", uuid}
    };

    std::string selectQuery = QueryBuilder::buildSelect(
                                  database.getDatabaseType(),
                                  SOURCES_TABLE_NAME,
    { FIELD_SOURCES_ID },
    filters,
    "",  // no order
    1    // limit 1
                              );

    auto result = database.query(selectQuery, { uuid });
    return !result.empty() ? std::stoi(result[0][FIELD_SOURCES_ID]) : SOURCE_NOT_FOUND;
}
#endif
