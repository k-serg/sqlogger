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

#include <cassert>
#include <iostream>
#include <thread>
#include <filesystem>
#include "log_manager.h"

#ifdef USE_AES
    #pragma message("AES support enabled.")
#endif

#ifdef USE_SOURCE_INFO
    #pragma message("SOURCE INFO support enabled.")
    #include <3rdparty/stduuid/include/uuid.h>
#endif

// Set "USE_MYSQL" option to "ON" in CMake parameters, to enable MySQL support.
#ifdef USE_MYSQL
    #pragma message("MySQL support enabled.")
#endif

// Set "USE_POSTGRESQL" option to "ON" in CMake parameters, to enable PostgreSQL support.
#ifdef USE_POSTGRESQL
    #pragma message("PostgreSQL support enabled.")
#endif

// Set "USE_MONGODB" option to "ON" in CMake parameters, to enable MongoDB support.
#ifdef USE_MONGODB
    #pragma message("MongoDB support enabled.")
#endif

// Test Config params.
constexpr auto TEST_LOGGER_NAME = "test_logger";
constexpr auto TEST_NUM_THREADS = 4;
constexpr auto TEST_USE_SYNC_MODE = true;
constexpr auto TEST_ONLY_FILE_NAME = true;
constexpr auto TEST_USE_BATCH = true;
constexpr auto TEST_BATCH_SIZE = 1000;
constexpr LogLevel TEST_LOG_LEVEL = LogLevel::Trace;

// Test params.
constexpr auto TEST_EXPORT_FILE = "test_logs_export";
constexpr auto TEST_PERFORMANCE_TEST_ENTRIES = 100;
constexpr auto TEST_PRINT_LOGS = false;
constexpr auto TEST_WAIT_UNTIL_EMPTY_MSEC = 1000;
constexpr auto TEST_STRESS_NUM_CONNECTIONS = 10;
constexpr auto TEST_STRESS_LOGS_PER_CONNECTION = 1000;

// Test database params.
constexpr auto TEST_DATABASE_NAME = "test_logs";
constexpr auto TEST_DATABASE_TABLE = "logs";
constexpr auto TEST_DATABASE_HOST = "localhost";
constexpr auto TEST_DATABASE_USER = "test";
constexpr auto TEST_DATABASE_PASS = "test";
constexpr auto TEST_ENC_DEC_PASS_KEY = "iknowyoursecrets";
constexpr auto TEST_ENC_DEC_STRING = "test_string";
constexpr auto TEST_DATABASE_FILE = "test_logs.db";

static LogConfig::Config testConfig;
static Logger* testLogger = nullptr;

#ifdef USE_SOURCE_INFO
// Test SourceInfo params.
constexpr auto TEST_SOURCE_NAME = "test_source";
constexpr auto TEST_SOURCE_UUID = "4472ab03-4184-44ab-921c-751a702c42ca";

SourceInfo TEST_SOURCE_INFO = [ & ]()
{
    SourceInfo sourceInfo;
    sourceInfo.sourceId = 0; // id will be obtained later from the database.
    sourceInfo.name = TEST_SOURCE_NAME;
    sourceInfo.uuid = TEST_SOURCE_UUID;
    return sourceInfo;
}
();
#endif

DataBaseType TEST_DATABASE_TYPE = [ & ]()
{
    return DataBaseHelper::stringToDatabaseType(DB_TYPE_STR_SQLITE);
}
();

int TEST_DATABASE_PORT = [ & ]()
{
    return DataBaseHelper::getDataBaseDefaultPort(TEST_DATABASE_TYPE);
}
();

static LogConfig::Config getDefaultConfig()
{
    LogConfig::Config config;
    config.name = TEST_LOGGER_NAME;
    config.syncMode = TEST_USE_SYNC_MODE;
    config.numThreads = TEST_NUM_THREADS;
    config.onlyFileNames = TEST_ONLY_FILE_NAME;
    config.minLogLevel = TEST_LOG_LEVEL;
    config.databaseName = TEST_DATABASE_FILE;
    config.databaseTable = TEST_DATABASE_TABLE;
    //config.databaseHost = TEST_DATABASE_HOST;
    //config.databasePort = TEST_DATABASE_PORT;
    //config.databaseUser = TEST_DATABASE_USER;
    //config.databasePass = TEST_DATABASE_PASS;
    //config.passKey = TEST_ENC_DEC_PASS_KEY;
    config.databaseType = TEST_DATABASE_TYPE;
    config.useBatch = TEST_USE_BATCH;
    config.batchSize = TEST_BATCH_SIZE;
#ifdef USE_SOURCE_INFO
    config.sourceUuid = TEST_SOURCE_UUID;
    config.sourceName = TEST_SOURCE_NAME;
#endif

    return config;
}

namespace
{
    const LogConfig::Config& getTestConfig(const std::string& configFile = LOG_DEFAULT_INI_FILENAME)
    {
        if(!testConfig.databaseType.has_value())
        {
            try
            {
                if(configFile.empty() || !std::filesystem::exists(configFile))
                {
                    testConfig = getDefaultConfig();
                }
                else
                {
                    testConfig = LogConfig::Config::loadFromINI(configFile, TEST_ENC_DEC_PASS_KEY);

                    LogConfig::ValidateResult result = testConfig.validate();
                    if(!result.ok())
                    {
                        std::string err = result.print();
                        throw std::runtime_error(err);
                    }
                }
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << std::endl;
                testConfig = getDefaultConfig();
            }
        }
        return testConfig;
    }

    Logger& getTestLogger(const LogConfig::Config& config)
    {
        if(!testLogger)
        {
            testLogger = & LogManager::getInstance().createLogger(
                             config.name.value(),
                             config
#ifdef USE_SOURCE_INFO
                             , SourceInfo { 0, // id will be obtained later from the database.
                                            config.sourceUuid.value(),
                                            config.sourceName.value()
                                          }
#endif
                         );
        }
        return *testLogger;
    }
}

void clearLogs(
#ifdef USE_SOURCE_INFO
    const bool clearSources = true
#endif
)
{
    Logger& logger = getTestLogger(getTestConfig());
    logger.clearLogs(
#ifdef USE_SOURCE_INFO
              clearSources
#endif
          );
}

/**
 * @brief Load config from file.
 * @param filename File name to load.
 * @return LogConfig::Config structure.
*/
LogConfig::Config loadConfig(const std::string& filename, const std::string& passKey)
{
    LogConfig::Config config;
    try
    {
        config = LogConfig::Config::loadFromINI(filename, passKey);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return config;
}

/**
 * @brief Saves config into file.
 * @param filename File name to save.
 * @param config LogConfig::Config structure.
*/
void saveConfig(const LogConfig::Config& config, const std::string& filename)
{
    LogConfig::Config::saveToINI(config, filename);
}

void showMessage(const std::string& msg)
{
    std::cout << msg << std::endl;
}

/**
 * @brief Print logs and it size into stdout
 * @param logs LogEntryList
*/
void printLogs(const LogEntryList& logs)
{
    if(!TEST_PRINT_LOGS) return;

    std::stringstream ss;

    ss << "Logs size: " << logs.size() << std::endl;
    for(const auto & log : logs)
    {
        ss << log.print() << std::endl;
    }

    showMessage(ss.str());
}

/**
 * @brief Test basic logging functionality.
 */
void testBasicFunctionality()
{
    std::string testName = "Basic functionality test";
    showMessage(testName + " started...");

    Logger& logger = getTestLogger(getTestConfig());

    // Clear the database before starting the test
    logger.clearLogs();

    const std::string infoMsg("This is an info message");
    const std::string warningMsg("This is a warning message");
    const std::string errorMsg("This is an error message");

    // Log some messages
    LOG_INFO(logger) << infoMsg;
    LOG_WARNING(logger) << warningMsg;
    LOG_ERROR(logger) << errorMsg;

    // Wait until all logs are written
    if(!logger.waitUntilEmpty(std::chrono::milliseconds(TEST_WAIT_UNTIL_EMPTY_MSEC)))
    {
        std::cerr << "Timeout while waiting for task queue to empty" << std::endl;
    }

    // flush batched logs for testing purposes
    logger.flush();

    // Retrieve all logs
    LogEntryList allLogs = logger.getAllLogs();

    // Debug output (TEST_PRINT_LOGS == 1)
    printLogs(allLogs);

    assert(allLogs.size() == 3); // All three messages must be written

    // Check log messages without assuming order
    bool infoFound = false, warningFound = false, errorFound = false;
    for(const auto & log : allLogs)
    {
        if(log.level == LOG_LEVEL_INFO && log.message == infoMsg) infoFound = true;
        if(log.level == LOG_LEVEL_WARNING && log.message == warningMsg) warningFound = true;
        if(log.level == LOG_LEVEL_ERROR && log.message == errorMsg) errorFound = true;
    }
    assert(infoFound && warningFound && errorFound);

    showMessage(testName + " passed!\n");
}

/**
 * @brief Test filtering logs by log level.
 */
void testFilterByLevel()
{
    std::string testName = "Filter by level test";
    showMessage(testName + " started...");

    Logger& logger = getTestLogger(getTestConfig());

    // Clear the database before starting the test
    logger.clearLogs();

    const auto level = LogLevel::Info;
    const std::string msg("Level-specific message");

    // Log a message
    LOG_INFO(logger) << msg;

    // Wait until all logs are written
    if(!logger.waitUntilEmpty(std::chrono::milliseconds(TEST_WAIT_UNTIL_EMPTY_MSEC)))
    {
        std::cerr << "Timeout while waiting for task queue to empty" << std::endl;
    }

    // flush batched logs for testing purposes
    logger.flush();

    // Retrieve logs for the specified level
    LogEntryList levelLogs = logger.getLogsByLevel(level);

    printLogs(levelLogs);

    assert(levelLogs.size() == 1); // One entry for the level
    assert(levelLogs[0].message == msg);

    showMessage(testName + " passed!\n");
}

/**
 * @brief Test filtering logs by thread ID.
 */
void testFilterByThreadId()
{
    std::string testName = "Filter By ThreadId test";
    showMessage(testName + " started...");

    Logger& logger = getTestLogger(getTestConfig());

    // Clear the database before starting the test
    logger.clearLogs();

    const std::string msg("Thread-specific message");

    // Log a message in the current thread
    LOG_INFO(logger) << msg;

    // Get the current thread ID
    std::string currentThreadId = LogHelper::threadIdToString(std::this_thread::get_id());

    // Wait until all logs are written
    if(!logger.waitUntilEmpty(std::chrono::milliseconds(TEST_WAIT_UNTIL_EMPTY_MSEC)))
    {
        std::cerr << "Timeout while waiting for task queue to empty" << std::endl;
    }

    // flush batched logs for testing purposes
    logger.flush();

    // Retrieve logs for the current thread
    LogEntryList threadLogs = logger.getLogsByThreadId(currentThreadId);

    printLogs(threadLogs);

    assert(threadLogs.size() == 1); // At least one entry for the current thread
    assert(threadLogs[0].message == msg);

    showMessage(testName + " passed!\n");
}

/**
 * @brief Test filtering logs by filename.
 */
void testFilterByFile()
{
    std::string testName = "Filter By File test";
    showMessage(testName + " started...");

    Logger& logger = getTestLogger(getTestConfig());

    // Clear the database before starting the test
    logger.clearLogs();

    const std::string msg("File-specific message");

    // Log a message
    LOG_INFO(logger) << msg;

    // Wait until all logs are written
    if(!logger.waitUntilEmpty(std::chrono::milliseconds(TEST_WAIT_UNTIL_EMPTY_MSEC)))
    {
        std::cerr << "Timeout while waiting for task queue to empty" << std::endl;
    }

    std::string file;

    // Retrieve logs for the current file
    if(testConfig.onlyFileNames.value() == true)
        file = std::filesystem::path(__FILE__).filename().string();
    else
        file = __FILE__;

    // flush batched logs for testing purposes
    logger.flush();

    LogEntryList fileLogs = logger.getLogsByFile(file);

    printLogs(fileLogs);

    assert(fileLogs.size() >= 1); // At least one entry for the file
    assert(fileLogs[0].message == msg);

    showMessage(testName + " passed!\n");
}

/**
 * @brief Test filtering logs by function name.
 */
void testFilterByFunction()
{
    std::string testName = "Filter By Function test";
    showMessage(testName + " started...");

    Logger& logger = getTestLogger(getTestConfig());

    // Clear the database before starting the test
    logger.clearLogs();

    const std::string msg("Function-specific message");

    // Log a message
    LOG_INFO(logger) << msg;

    // Wait until all logs are written
    if(!logger.waitUntilEmpty(std::chrono::milliseconds(TEST_WAIT_UNTIL_EMPTY_MSEC)))
    {
        std::cerr << "Timeout while waiting for task queue to empty" << std::endl;
    }

    // flush batched logs for testing purposes
    logger.flush();

    // Retrieve logs for the current function
    LogEntryList functionLogs = logger.getLogsByFunction(__func__);

    printLogs(functionLogs);

    assert(functionLogs.size() == 1); // One entry for the function
    assert(functionLogs[0].message == msg);

    showMessage(testName + " passed!\n");
}

/**
 * @brief Test filtering logs by timestamp range.
 */
void testFilterByTimestampRange()
{
    std::string testName = "Filter By Timestamp Range test";
    showMessage(testName + " started...");

    Logger& logger = getTestLogger(getTestConfig());

    // Clear the database before starting the test
    logger.clearLogs();

    // Log a message
    const std::string msg("Timestamp-specific message");
    LOG_INFO(logger) << msg;

    // Get the current timestamp
    std::string currentTime = LogHelper::getCurrentTimestamp();

    // Wait until all logs are written
    if(!logger.waitUntilEmpty(std::chrono::milliseconds(TEST_WAIT_UNTIL_EMPTY_MSEC)))
    {
        std::cerr << "Timeout while waiting for task queue to empty" << std::endl;
    }

    // flush batched logs for testing purposes
    logger.flush();

    // Retrieve logs within the specified timestamp range
    LogEntryList timeLogs = logger.getLogsByTimestampRange("1970-01-01 00:00:00", currentTime);

    printLogs(timeLogs);

    assert(timeLogs.size() >= 1); // At least one entry
    assert(timeLogs[0].message == msg);

    showMessage(testName + " passed!\n");
}

/**
 * @brief Test clearing logs from the database.
 */
void testClearLogs()
{
    std::string testName = "Clear Logs test";
    showMessage(testName + " started...");

    Logger& logger = getTestLogger(getTestConfig());

    // Clear the database before starting the test
    logger.clearLogs();

    // Log a message
    LOG_INFO(logger) << "Message to be cleared";

    // Wait until all logs are written
    if(!logger.waitUntilEmpty(std::chrono::milliseconds(TEST_WAIT_UNTIL_EMPTY_MSEC)))
    {
        std::cerr << "Timeout while waiting for task queue to empty" << std::endl;
    }

    // flush batched logs for testing purposes
    logger.flush();

    if(!logger.waitUntilEmpty(std::chrono::milliseconds(TEST_WAIT_UNTIL_EMPTY_MSEC)))
    {
        std::cerr << "Timeout while waiting for task queue to empty" << std::endl;
    }

    // Clear the database again
    logger.clearLogs();

    // Check that logs have been cleared
    LogEntryList allLogs = logger.getAllLogs();
    assert(allLogs.empty());

    showMessage(testName + " passed!\n");
}

/**
 * @brief Test logging from multiple threads.
 */
void testMultiThread()
{
    std::string testName = "Multi Thread test";
    showMessage(testName + " started...");

    Logger& logger = getTestLogger(getTestConfig());

    logger.clearLogs();

    const int numThreads = 10;
    const int logsPerThread = 100;
    std::vector<std::thread> threads;
    std::atomic<int> logsWritten{ 0 };
    std::mutex logMutex;

    auto threadFunc = [ & ](int threadId)
    {
        try
        {
            for(int j = 0; j < logsPerThread; ++j)
            {
                {
                    std::lock_guard<std::mutex> lock(logMutex);
                    LOG_INFO(logger) << "Thread " << threadId << ", log " << j;
                }
                logsWritten++;
            }
        }
        catch(const std::exception& e)
        {
            std::cerr << "Exception in thread " << threadId << ": " << e.what() << std::endl;
        }
    };

    // Create threads
    for(int i = 0; i < numThreads; ++i)
    {
        threads.emplace_back(threadFunc, i);
    }

    // Wait for all threads to complete
    for(auto & thread : threads)
    {
        if(thread.joinable())
        {
            thread.join();
        }
    }

    assert(logsWritten == numThreads* logsPerThread);

    // Wait until all logs are written
    if(!logger.waitUntilEmpty(std::chrono::milliseconds(TEST_WAIT_UNTIL_EMPTY_MSEC * 5)))
    {
        std::cerr << "Warning: Task queue timeout" << std::endl;
    }

    // flush batched logs for testing purposes
    logger.flush();

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    LogEntryList allLogs = logger.getAllLogs();

    if(allLogs.size() != numThreads* logsPerThread)
    {
        std::cerr << "Error: Expected " << numThreads* logsPerThread << " logs, got " << allLogs.size() << std::endl;
    }

    printLogs(allLogs);

    assert(allLogs.size() == numThreads* logsPerThread);

    showMessage(testName + " passed!\n");
}

/**
 * @brief Test Multi-filters log entries retrieve.
*/
void testMultiFilters()
{
    std::string testName = "Multi Filters test";
    showMessage(testName + " started...");

    Logger& logger = getTestLogger(getTestConfig());

    // Clear the database before starting the test
    logger.clearLogs();

    const std::string infoMsg("This is an info message");
    const std::string warningMsg("This is a warning message");
    const std::string errorMsg("This is an error message");

    LOG_INFO(logger) << infoMsg;
    LOG_WARNING(logger) << warningMsg;
    LOG_ERROR(logger) << errorMsg;

    // Wait until all logs are written
    if(!logger.waitUntilEmpty(std::chrono::milliseconds(TEST_WAIT_UNTIL_EMPTY_MSEC)))
    {
        std::cerr << "Timeout while waiting for task queue to empty" << std::endl;
    }

    // Define filters
    std::vector<Filter> filters;

    // Filter by log level (only Error messages)
    Filter levelFilter;
    levelFilter.type = Filter::Type::Level;
    levelFilter.field = levelFilter.typeToField();
    levelFilter.value = levelToString(LogLevel::Error); // "ERROR"
    levelFilter.op = "=";
    filters.push_back(levelFilter);

    // Filter by file name (only logs from this file: "test_logger.cpp")
    Filter fileFilter;
    fileFilter.type = Filter::Type::File;
    fileFilter.field = fileFilter.typeToField();

    std::string file;

    // Retrieve logs for the current file
    if(testConfig.onlyFileNames.value() == true)
        fileFilter.value = std::filesystem::path(__FILE__).filename().string();
    else
        fileFilter.value = __FILE__;

    fileFilter.op = "=";
    filters.push_back(fileFilter);

    // Filter by timestamp range (logs between two timestamps)
    // Get the current timestamp
    std::string currentTime = LogHelper::getCurrentTimestamp();

    Filter timestampFilterStart;
    timestampFilterStart.type = Filter::Type::TimestampRange;
    timestampFilterStart.field = timestampFilterStart.typeToField(); // "timestamp"
    timestampFilterStart.value = "1970-01-01 00:00:00"; // Start timestamp
    timestampFilterStart.op = ">=";
    filters.push_back(timestampFilterStart);

    Filter timestampFilterEnd;
    timestampFilterEnd.type = Filter::Type::TimestampRange;
    timestampFilterEnd.field = timestampFilterEnd.typeToField(); // "timestamp"
    timestampFilterEnd.value = currentTime; // End timestamp
    timestampFilterEnd.op = "<=";
    filters.push_back(timestampFilterEnd);

    // flush batched logs for testing purposes
    logger.flush();

    // Retrieve logs using the filters
    LogEntryList filteredLogs = logger.getLogsByFilters(filters);

    printLogs(filteredLogs);

    assert(filteredLogs.size() == 1);
    assert(filteredLogs[0].message == errorMsg);

    showMessage(testName + " passed!\n");
}

/**
 * @brief Test exporting logs to a file.
 */
void testFileExport()
{
    std::string testName = "File Export test";
    showMessage(testName + " started...");

    Logger& logger = getTestLogger(getTestConfig());

    // Clear the database before starting the test
    logger.clearLogs();

    const std::string infoMsg("This is an info message");
    const std::string warningMsg("This is a warning message");
    const std::string errorMsg("This is an error message");

    // Log some messages
    LOG_INFO(logger) << infoMsg;
    LOG_WARNING(logger) << warningMsg;
    LOG_ERROR(logger) << errorMsg;

    // Wait until all logs are written
    if(!logger.waitUntilEmpty(std::chrono::milliseconds(TEST_WAIT_UNTIL_EMPTY_MSEC)))
    {
        std::cerr << "Timeout while waiting for task queue to empty" << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Additional delay

    // flush batched logs for testing purposes
    logger.flush();

    // Retrieve all logs
    LogEntryList allLogs = logger.getAllLogs();

    printLogs(allLogs);

    assert(allLogs.size() == 3);

    const std::filesystem::path path(std::filesystem::absolute(TEST_EXPORT_FILE));
    std::ofstream outFile(path.string() + ".txt");
    const std::string delimiter(" ");

    // Test single entry print to file
    for(const auto & entry : allLogs)
    {
        entry.printToFile(outFile, delimiter, false);
    }

    // Close the file stream
    outFile.close();

    // Test static export methods for the entry list
    Logger::exportTo(path.string() + ".txt", LogExport::Format::TXT, allLogs, delimiter, false);
    Logger::exportTo(path.string() + ".csv", LogExport::Format::CSV, allLogs);
    Logger::exportTo(path.string() + ".xml", LogExport::Format::XML, allLogs);
    Logger::exportTo(path.string() + ".json", LogExport::Format::JSON, allLogs);
    Logger::exportTo(path.string() + ".yaml", LogExport::Format::YAML, allLogs);

    // Unknown Format
    //Logger::exportTo(path.string() + ".bad", static_cast<LogExport::Format>(10), allLogs, delimiter, false);

    showMessage(testName + " passed!\n");
}

/**
 * @brief Remove test files created during testing.
 */
void removeTestFiles()
{
    const auto dbFilePath = std::filesystem::absolute(TEST_DATABASE_FILE);

    std::stringstream ss;

    ss << "Removing test files..." << std::endl;

    if(std::filesystem::exists(dbFilePath))
    {
        try
        {
            std::filesystem::remove(dbFilePath);
        }
        catch(const std::exception& e)
        {
            std::cout << "Can't remove database file: " << dbFilePath << "(" << e.what() << ")" << std::endl;
        }
    }

    const auto exportBasePath = std::filesystem::absolute(TEST_EXPORT_FILE);
    std::vector<std::filesystem::path> exportPaths;

    exportPaths.emplace_back(exportBasePath.string() + ".txt");
    exportPaths.emplace_back(exportBasePath.string() + ".csv");
    exportPaths.emplace_back(exportBasePath.string() + ".xml");
    exportPaths.emplace_back(exportBasePath.string() + ".json");
    exportPaths.emplace_back(exportBasePath.string() + ".yaml");

    for(const auto & exportPath : exportPaths)
    {
        if(std::filesystem::exists(exportPath))
        {
            try
            {
                std::filesystem::remove(exportPath);
            }
            catch(const std::exception& e)
            {
                std::cout << "Can't remove export output file: " << exportPath << "(" << e.what() << ")" << std::endl;
            }
        }
    }

    const auto configFilePath = std::filesystem::absolute(LOG_DEFAULT_INI_FILENAME);

    if(std::filesystem::exists(configFilePath))
    {
        try
        {
            std::filesystem::remove(configFilePath);
        }
        catch(const std::exception& e)
        {
            std::cout << "Can't remove config file: " << configFilePath << "(" << e.what() << ")" << std::endl;
        }
    }

    showMessage(ss.str());
}

/**
 * @brief Test the performance of the logger.
 */
void testPerformance()
{
    std::string testName = "Performance test";
    showMessage(testName + " started...");

    Logger& logger = getTestLogger(getTestConfig());

    // Clear the database before starting the test
    logger.clearLogs();

    // Reset stats
    logger.resetStats();

    const int numLogs = TEST_PERFORMANCE_TEST_ENTRIES;
    auto startTime = std::chrono::high_resolution_clock::now();

    for(int i = 0; i < numLogs; ++i)
    {
        LOG_INFO(logger) << "Log message " << i;
    }

    // Wait until all logs are written
    if(!logger.waitUntilEmpty(std::chrono::milliseconds(TEST_WAIT_UNTIL_EMPTY_MSEC)))
    {
        std::cerr << "Timeout while waiting for task queue to empty" << std::endl;
    }

    // flush batched logs for testing purposes
    logger.flush();

    if(!logger.waitUntilEmpty(std::chrono::milliseconds(TEST_WAIT_UNTIL_EMPTY_MSEC)))
    {
        std::cerr << "Timeout while waiting for task queue to empty" << std::endl;
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    auto stats = logger.getStats();

    std::stringstream ss;

    ss << std::endl << "*** Performance Test ***" << std::endl;
    ss << "Logged " << numLogs << " messages in " << duration << " ms" << std::endl;

    // Get Stats
    ss << std::endl << Logger::getFormattedStats(stats) << std::endl;

    showMessage(ss.str());

    showMessage(testName + " passed!\n");
}

/**
 * @brief Test config save/load.
*/
void testConfigSaveLoad()
{
    std::string testName = "Config Save Load test";
    showMessage(testName + " started...");

    LogConfig::Config config = getDefaultConfig();

    saveConfig(config, LOG_DEFAULT_INI_FILENAME);

    auto loadedConfig = loadConfig(LOG_DEFAULT_INI_FILENAME, TEST_ENC_DEC_PASS_KEY);

    assert(loadedConfig.name == config.name);
    assert(loadedConfig.syncMode == config.syncMode);
    assert(loadedConfig.numThreads == config.numThreads);
    assert(loadedConfig.onlyFileNames == config.onlyFileNames);
    assert(loadedConfig.minLogLevel == config.minLogLevel);
    assert(loadedConfig.databaseName == config.databaseName);
    assert(loadedConfig.databaseTable == config.databaseTable);
    assert(loadedConfig.databaseHost == config.databaseHost);
    assert(loadedConfig.databasePort == config.databasePort);
    assert(loadedConfig.databaseUser == config.databaseUser);
    assert(loadedConfig.databasePass == config.databasePass);
    assert(loadedConfig.databaseType == config.databaseType);
    assert(loadedConfig.useBatch == config.useBatch);
    assert(loadedConfig.batchSize == config.batchSize);
#ifdef USE_SOURCE_INFO
    assert(loadedConfig.sourceUuid == config.sourceUuid);
    assert(loadedConfig.sourceName == config.sourceName);
#endif

    showMessage(testName + " passed!\n");
}

/**
 * @brief Test logging with SourceInfo and filtering by source.
 */
void testSourceInfo()
{
    std::string testName = "Source Info test";

#ifdef USE_SOURCE_INFO
    showMessage(testName + " started...");

    Logger& logger = getTestLogger(getTestConfig());

    // Clear the database before starting the test
    logger.clearLogs(true);

    // Add the source to the database and get its sourceId
    // Use TEST_SOURCE_INFO, which is already defined
    int sourceId = logger.addSource(testConfig.sourceName.value(), testConfig.sourceUuid.value());
    assert(sourceId != SOURCE_NOT_FOUND); // Ensure the source was added successfully

    // Use TEST_SOURCE_INFO, which is already defined
    const std::string msg = "This is a message from " + testConfig.sourceName.value();

    // Log a message with this source
    LOG_INFO(logger) << msg;

    // Wait until all logs are written
    if(!logger.waitUntilEmpty(std::chrono::milliseconds(TEST_WAIT_UNTIL_EMPTY_MSEC)))
    {
        std::cerr << "Timeout while waiting for task queue to empty" << std::endl;
    }

    // flush batched logs for testing purposes
    logger.flush();

    // Retrieve logs for the specific sourceId
    LogEntryList sourceIdLogs = logger.getLogsBySourceId(sourceId);

    printLogs(sourceIdLogs);

    assert(sourceIdLogs.size() == 1); // Ensure only one log entry is found
    assert(sourceIdLogs[0].message == msg); // Ensure the message matches

    // Retrieve logs for the specific sourceUuid
    LogEntryList sourceUuidLogs = logger.getLogsBySourceUuid(testConfig.sourceUuid.value());

    printLogs(sourceUuidLogs);

    assert(sourceUuidLogs.size() == 1); // Ensure only one log entry is found
    assert(sourceUuidLogs[0].message == msg); // Ensure the message matches

    showMessage(testName + " passed!\n");
#else
    showMessage(testName + " skipped (USE_SOURCE_INFO not defined).");
#endif
}

/**
 * @brief Test retrieving all sources.
 */
void testGetAllSources()
{
    std::string testName = "Get All Sources test";

#ifdef USE_SOURCE_INFO
    showMessage(testName + " started...");

    Logger& logger = getTestLogger(getTestConfig());

    // Clear the database and sources before starting the test
    logger.clearLogs(true);

    // Add the source to the database and get its sourceId
    // Use TEST_SOURCE_INFO, which is already defined
    int sourceId = logger.addSource(testConfig.sourceName.value(), testConfig.sourceUuid.value());
    assert(sourceId != SOURCE_NOT_FOUND); // Ensure the source was added successfully

    auto allSources = logger.getAllSources();
    assert(allSources.size() == 1); // Ensure only one source is found
    assert(allSources[0].uuid == testConfig.sourceUuid.value()); // Ensure the UUID matches
    assert(allSources[0].name == testConfig.sourceName.value()); // Ensure the name matches

    showMessage(testName + " passed!\n");
#else
    showMessage(testName + " skipped (USE_SOURCE_INFO not defined).");
#endif
}

/**
 * @brief Test retrieving source information by UUID.
 */
void testGetSourceByUuid()
{
    std::string testName = "Get Source By Uuid test";


#ifdef USE_SOURCE_INFO
    showMessage(testName + " started...");

    Logger& logger = getTestLogger(getTestConfig());

    // Clear the database before starting the test
    logger.clearLogs(true);

    // Add the source to the database and get its sourceId
    // Use TEST_SOURCE_INFO, which is already defined
    int sourceId = logger.addSource(testConfig.sourceName.value(), testConfig.sourceUuid.value());
    assert(sourceId != SOURCE_NOT_FOUND); // Ensure the source was added successfully

    // Retrieve the source information by UUID
    auto retrievedSource = logger.getSourceByUuid(testConfig.sourceUuid.value());
    assert(retrievedSource.has_value()); // Ensure the source was found
    assert(retrievedSource->uuid == testConfig.sourceUuid.value()); // Ensure the UUID matches
    assert(retrievedSource->name == testConfig.sourceName.value()); // Ensure the name matches

    showMessage(testName + " passed!\n");
#else
    showMessage(testName + " skipped (USE_SOURCE_INFO not defined).");
#endif
}

void testEncryptDecrypt()
{
    std::string testName = "Encrypt Decrypt test";
    showMessage(testName + " started...");

    const std::string sourceStr = TEST_ENC_DEC_STRING;
    const std::string encodedStr = LogCrypto::encrypt(sourceStr, TEST_ENC_DEC_PASS_KEY);
    const std::string decodedStr = LogCrypto::decrypt(encodedStr, TEST_ENC_DEC_PASS_KEY);
    assert(sourceStr == decodedStr);

    showMessage(testName + " passed!\n");
}

/**
 * @brief Stress test with multiple connections having different Source Info
 * @note Only for MySQL/PostgreSQL
 */
void testMultiConnectionStress()
{
    std::string testName = "Multi-Connection Stress test";
    showMessage(testName + " started...");

    Logger& logger = getTestLogger(getTestConfig());

    switch(testConfig.databaseType.value())
    {
        case DataBaseType::MySQL:
        case DataBaseType::PostgreSQL:
        case DataBaseType::MongoDB:
            break;

        case DataBaseType::Mock:
        case DataBaseType::SQLite:
        default:
        {
            std::cout << std::endl << "Skipping multi-connection test" << std::endl;
            return;
        }
    }

    const int numConnections = TEST_STRESS_NUM_CONNECTIONS; // Number of connections with unique sources
    const int logsPerConnection = TEST_STRESS_LOGS_PER_CONNECTION; // Logs per connection
    std::vector<std::thread> threads;
    std::atomic<int> logsWritten{ 0 };

    // Clear existing logs
    logger.clearLogs(
#ifdef USE_SOURCE_INFO
              true
#endif
          );

    std::string loggerNamePrefix = "stress_logger_";
    std::string sourceNamePrefix = "stress_source_";

    // Thread function with unique Source Info
    auto writerFunc = [ & ](int connId)
    {
        // Create unique config per connection
        LogConfig::Config config = getTestConfig();

        auto connString = LogConfig::configToConnectionString(config);

        std::string loggerName = loggerNamePrefix + std::to_string(connId);
        config.name = loggerName;

#ifdef USE_SOURCE_INFO
        // Generate unique source info
        SourceInfo srcInfo;
        srcInfo.name = sourceNamePrefix + std::to_string(connId);
        srcInfo.uuid = generateUUID();

        // Create logger with unique source
        Logger& connLogger = LogManager::getInstance().createLogger(loggerName, config, srcInfo);
#else
        Logger& connLogger = LogManager::getInstance().createLogger(loggerName, config);
#endif

        // Write logs
        for(int i = 0; i < logsPerConnection; ++i)
        {
            LOG_INFO(connLogger) << "Stress log " << i << " from source " << connId;
            logsWritten++;
        }
    };

    // Start threads
    for(int i = 0; i < numConnections; ++i)
    {
        threads.emplace_back(writerFunc, i);
    }

    // Wait for completion
    for(auto & t : threads)
    {
        if(t.joinable()) t.join();
    }

    // Verification
    logger.waitUntilEmpty(std::chrono::milliseconds(TEST_WAIT_UNTIL_EMPTY_MSEC));

#ifdef USE_SOURCE_INFO
    // Verify source-specific logging
    for(int i = 0; i < numConnections; ++i)
    {
        std::string sourceName = sourceNamePrefix + std::to_string(i);
        auto srcInfo = logger.getSourceByName(sourceName);

        if(srcInfo.has_value())
        {
            auto logs = logger.getLogsBySourceUuid(srcInfo->uuid);
            // std::cout << "Source " << sourceName << " logs: " << logs.size() << std::endl;
            assert(logs.size() == logsPerConnection);
        }
    }
#endif

    // Verify total logs
    auto allLogs = logger.getAllLogs();

    std::stringstream ss;
    ss << std::endl << "*** Multi-Connection Stress Test ***" << std::endl;
    ss << "Number of connections: " << numConnections << std::endl;
    ss << "Logs per connection: " << logsPerConnection << std::endl;
    ss << "Expected logs: " << (numConnections* logsPerConnection) << std::endl;
    ss << "Total logs: " << allLogs.size() << std::endl;

    showMessage(ss.str());

    assert(allLogs.size() == numConnections* logsPerConnection);

    // Shutdown stress test loggers by it's name
    int removed = LogManager::getInstance().removeIf([ & ](const auto& name, const auto& logger)
    {
        return logger.getConfig().name.value().find(loggerNamePrefix) != std::string::npos;
    });

    showMessage(testName + " passed!\n");
}

void testLimitOffset()
{
    std::string testName = "Limit-Offset test";
    showMessage(testName + " started...");

    Logger& logger = getTestLogger(getTestConfig());

    // Clear the database before starting the test
    logger.clearLogs();
    const int numLogs = 10;
    const int limit = 3;
    const int offset = 5;
    std::vector<Filter> filters = {}; // No filters

    for(int i = 0; i < numLogs; ++i)
    {
        LOG_INFO(logger) << "Log " << i;
    }

    // Wait until all logs are written
    if(!logger.waitUntilEmpty(std::chrono::milliseconds(TEST_WAIT_UNTIL_EMPTY_MSEC)))
    {
        std::cerr << "Timeout while waiting for task queue to empty" << std::endl;
    }

    // flush batched logs for testing purposes
    logger.flush();

    // Retrieve all logs
    LogEntryList allLogs = logger.getAllLogs();

    // Retrieve logs using the limit and offset
    LogEntryList limitedLogs = logger.getAllLogs(limit, offset);

    printLogs(limitedLogs);

    assert(limitedLogs.size() == limit);
    assert(allLogs.size() == numLogs);

    // Compare messages
    assert(limitedLogs[0].message == allLogs[offset].message);
    assert(limitedLogs[1].message == allLogs[offset + 1].message);
    assert(limitedLogs[2].message == allLogs[offset + 2].message);

    showMessage(testName + " passed!\n");
}

/**
 * @brief Cleanup function to shut down the logger.
 */
void cleanup()
{
    std::stringstream ss;
    ss << "Shutdown " << (LogManager::getInstance().getCount() > 1
                          ? "loggers: "
                          : "logger: ");

    auto loggersConfig = LogManager::getInstance().getAllLoggersConfigs();

    std::vector<std::string> loggers;

    for(auto & [name, config] : loggersConfig)
    {
        loggers.emplace_back(name);
    }

    ss << StringHelper::join(loggers, ", ");

    showMessage(ss.str());
    LogManager::getInstance().removeAllLoggers();
}

int main(int argc, char* argv[])
{
    // Parse args
    std::string configFile = [ & ]()
    {
        if(argc > 1)
        {
            std::string arg = argv[1];
            if(arg == "-h" || arg == "--help")
            {
                std::cout << SQLOGGER_DESCRIPTION << std::endl;
                std::cout << "Usage: " << argv[0] << " [config.ini]" << std::endl;
                exit(0);
            }
            else if(arg == "-v" || arg == "--version")
            {
                std::cout << SQLOGGER_PROJECT_NAME << " v." << SQLOGGER_VERSION_FULL << std::endl;
                exit(0);
            }
            return arg;
        }
        return std::string();
    }
    ();

    std::cout << SQLOGGER_PROJECT_NAME << " v." << SQLOGGER_VERSION_FULL << std::endl;

    if(!configFile.empty())
        std::cout << "Test config file: " << configFile << std::endl;

    LogConfig::Config config = getTestConfig(configFile);

    DataBaseType requestedDb = config.databaseType.value();

    if(!DataBaseHelper::isDataBaseSupported(requestedDb))
    {
        std::cerr << "Requested database type " << DataBaseHelper::databaseTypeToString(requestedDb) <<  " not supported in this build";
        return 1;
    }

    // Create logger
    Logger& logger = getTestLogger(config);

    std::string TEST_CONN_STRING =  LogConfig::configToConnectionString(config);

    // Getting database type
    DataBaseType dbType = logger.getDataBaseType();

    // Getting string representation of the database type
    std::stringstream dbTypeString;
    dbTypeString << DataBaseHelper::databaseTypeToString(dbType);

    std::cout << "Tested on: ";

    switch(dbType)
    {
        case DataBaseType::Mock:
            dbTypeString << " data";
            break;

        case DataBaseType::SQLite:
            dbTypeString << " database" << std::endl << "Database file: " << config.databaseName.value();
            break;

        case DataBaseType::MySQL:
        case DataBaseType::PostgreSQL:
        case DataBaseType::MongoDB:
            dbTypeString << " database" << std::endl << "Connection string: " << TEST_CONN_STRING;
            break;

        default:
            break;
    }

    std::cout << dbTypeString.str() << std::endl;

    // Getting logger name
    std::string loggerName = logger.getName();
    std::cout << "Logger name: " << loggerName << std::endl;

    // Getting sync mode
    bool isSyncMode = logger.isSyncMode();
    std::cout << "Sync Mode: " << (isSyncMode ? "ON" : "OFF") << std::endl;
    std::cout << (isSyncMode ? "" : std::string("Number of threads: " + std::to_string(logger.getNumThreads()) + "\n"));

    // Getting use batch
    bool useBatch = logger.isBatchEnabled();
    std::cout << "Batch Insert: " << (useBatch ? "ON" : "OFF") << std::endl;
    std::cout << (useBatch ?  std::string("Batch size: " + std::to_string(logger.getBatchSize()) + "\n") : "");

#ifdef USE_SOURCE_INFO
    std::cout << "Source Info: ON" << std::endl;
#else
    std::cout << "Source Info: OFF" << std::endl;
#endif

#ifdef USE_AES
    std::cout << "AES Support: ON" << std::endl;
#else
    std::cout << "AES Support: OFF" << std::endl;
#endif

    std::cout << std::endl;

    try
    {
        testEncryptDecrypt();
        testConfigSaveLoad();
        testBasicFunctionality();
        testFilterByLevel();
        testFilterByFunction();
        testFilterByThreadId();
        testFilterByFile();
        testFilterByTimestampRange();
        testMultiFilters();
        testLimitOffset();
#ifdef USE_SOURCE_INFO
        testSourceInfo();
        testGetSourceByUuid();
        testGetAllSources();
#endif
        testMultiThread();
        testFileExport();
        testClearLogs();
        testPerformance();
        testMultiConnectionStress();

        std::cout << "All tests passed successfully!" << std::endl << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        clearLogs();
        cleanup();
        removeTestFiles();
    }
    catch(const std::exception& e)
    {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}