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
#include "sqlite_database.h"
#include "mock_database.h"
#include "logger.h"

#define TEST_DATABASE "test_logs.db"
#define TEST_EXPORT_FILE "test_logs_export"
#define TEST_ON_REAL_DB 1
#define TEST_NUM_THREADS 4
#define USE_SYNC_MODE 0
#define PERFORMANCE_TEST_ENTRIES 100
#define PRINT_LOGS 0
#define WAIT_UNTIL_EMPTY_MSEC 1000
#define ONLY_FILE_NAME 0

using namespace LogHelper;
using namespace LogConfig;

// Use a real database or mock
#if TEST_ON_REAL_DB == 1
    auto database = std::make_unique<SQLiteDatabase>(TEST_DATABASE);
#else
    auto database = std::make_unique<MockDatabase>();
#endif // TEST_ON_REAL_DB

// Minimum log level
constexpr Level LOG_LEVEL = Level::Trace;

// Config
Config config{ USE_SYNC_MODE, TEST_NUM_THREADS, ONLY_FILE_NAME };

Logger logger(std::move(database), config);

void printLogs(const LogEntryList logs)
{
#if PRINT_LOGS != 1
    return;
#endif
    std::cout << "Logs size: " << logs.size() << std::endl;
    for(const auto & log : logs)
    {
        std::cout << log.print() << std::endl;
    }
}

/**
 * @brief Test basic logging functionality.
 */
void testBasicFunctionality()
{
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
    if(!logger.waitUntilEmpty(std::chrono::milliseconds(WAIT_UNTIL_EMPTY_MSEC)))
    {
        std::cerr << "Timeout while waiting for task queue to empty" << std::endl;
    }

    // Retrieve all logs
    LogEntryList allLogs = logger.getAllLogs();

    // Debug output (PRINT_LOGS == 1)
    printLogs(allLogs);

    assert(allLogs.size() == 3); // All three messages must be written

    // Check log messages without assuming order
    bool infoFound = false, warningFound = false, errorFound = false;
    for(const auto & log : allLogs)
    {
        if(log.level == "INFO" && log.message == infoMsg) infoFound = true;
        if(log.level == "WARNING" && log.message == warningMsg) warningFound = true;
        if(log.level == "ERROR" && log.message == errorMsg) errorFound = true;
    }
    assert(infoFound && warningFound && errorFound);

    std::cout << "testBasicFunctionality passed!" << std::endl;
}

/**
 * @brief Test filtering logs by log level.
 */
void testFilterByLevel()
{
    // Clear the database before starting the test
    logger.clearLogs();

    const auto level = Level::Info;
    const std::string msg("Level-specific message");

    // Log a message
    LOG_INFO(logger) << msg;

    // Wait until all logs are written
    if(!logger.waitUntilEmpty(std::chrono::milliseconds(WAIT_UNTIL_EMPTY_MSEC)))
    {
        std::cerr << "Timeout while waiting for task queue to empty" << std::endl;
    }

    // Retrieve logs for the specified level
    LogEntryList levelLogs = logger.getLogsByLevel(level);

    printLogs(levelLogs);

    assert(levelLogs.size() == 1); // One entry for the level
    assert(levelLogs[0].message == msg);

    std::cout << "testFilterByLevel passed!" << std::endl;
}

/**
 * @brief Test filtering logs by thread ID.
 */
void testFilterByThreadId()
{
    // Clear the database before starting the test
    logger.clearLogs();

    const std::string msg("Thread-specific message");

    // Log a message in the current thread
    LOG_INFO(logger) << msg;

    // Get the current thread ID
    std::string currentThreadId = LogHelper::threadIdToString(std::this_thread::get_id());

    // Wait until all logs are written
    if(!logger.waitUntilEmpty(std::chrono::milliseconds(WAIT_UNTIL_EMPTY_MSEC)))
    {
        std::cerr << "Timeout while waiting for task queue to empty" << std::endl;
    }

    // Retrieve logs for the current thread
    auto threadLogs = logger.getLogsByThreadId(currentThreadId);

    printLogs(threadLogs);

    assert(threadLogs.size() == 1); // At least one entry for the current thread
    assert(threadLogs[0].message == msg);

    std::cout << "testFilterByThreadId passed!" << std::endl;
}

/**
 * @brief Test filtering logs by filename.
 */
void testFilterByFile()
{
    // Clear the database before starting the test
    logger.clearLogs();

    const std::string msg("File-specific message");

    // Log a message
    LOG_INFO(logger) << msg;

    // Wait until all logs are written
    if(!logger.waitUntilEmpty(std::chrono::milliseconds(WAIT_UNTIL_EMPTY_MSEC)))
    {
        std::cerr << "Timeout while waiting for task queue to empty" << std::endl;
    }

    // Retrieve logs for the current file
#if ONLY_FILE_NAME == 1
    const std::string file = std::filesystem::path(__FILE__).filename().string();
#else
    const std::string file = __FILE__;
#endif
    auto fileLogs = logger.getLogsByFile(file);

    printLogs(fileLogs);

    assert(fileLogs.size() >= 1); // At least one entry for the file
    assert(fileLogs[0].message == msg);

    std::cout << "testFilterByFile passed!" << std::endl;
}

/**
 * @brief Test filtering logs by function name.
 */
void testFilterByFunction()
{
    // Clear the database before starting the test
    logger.clearLogs();

    const std::string msg("Function-specific message");

    // Log a message
    LOG_INFO(logger) << msg;

    // Wait until all logs are written
    if(!logger.waitUntilEmpty(std::chrono::milliseconds(WAIT_UNTIL_EMPTY_MSEC)))
    {
        std::cerr << "Timeout while waiting for task queue to empty" << std::endl;
    }

    // Retrieve logs for the current function
    auto functionLogs = logger.getLogsByFunction(__func__);

    printLogs(functionLogs);

    assert(functionLogs.size() == 1); // One entry for the function
    assert(functionLogs[0].message == msg);

    std::cout << "testFilterByFunction passed!" << std::endl;
}

/**
 * @brief Test filtering logs by timestamp range.
 */
void testFilterByTimestampRange()
{
    // Clear the database before starting the test
    logger.clearLogs();

    // Log a message
    const std::string msg("Timestamp-specific message");
    LOG_INFO(logger) << msg;

    // Get the current timestamp
    std::string currentTime = LogHelper::getCurrentTimestamp();

    // Wait until all logs are written
    if(!logger.waitUntilEmpty(std::chrono::milliseconds(WAIT_UNTIL_EMPTY_MSEC)))
    {
        std::cerr << "Timeout while waiting for task queue to empty" << std::endl;
    }

    // Retrieve logs within the specified timestamp range
    auto timeLogs = logger.getLogsByTimestampRange("1970-01-01 00:00:00", currentTime);

    printLogs(timeLogs);

    assert(timeLogs.size() >= 1); // At least one entry
    assert(timeLogs[0].message == msg);

    std::cout << "testFilterByTimestampRange passed!" << std::endl;
}

/**
 * @brief Test clearing logs from the database.
 */
void testClearLogs()
{
    // Clear the database before starting the test
    logger.clearLogs();

    // Log a message
    LOG_INFO(logger) << "Message to be cleared";

    // Wait until all logs are written
    if(!logger.waitUntilEmpty(std::chrono::milliseconds(WAIT_UNTIL_EMPTY_MSEC)))
    {
        std::cerr << "Timeout while waiting for task queue to empty" << std::endl;
    }

    // Clear the database again
    logger.clearLogs();

    // Check that logs have been cleared
    LogEntryList allLogs = logger.getAllLogs();
    assert(allLogs.empty());

    std::cout << "testClearLogs passed!" << std::endl;
}

/**
 * @brief Test logging from multiple threads.
 */
void testMultiThread()
{
    // Clear the database before starting the test
    logger.clearLogs();

    const int numThreads = 10;
    const int logsPerThread = 100;
    std::vector<std::thread> threads;
    std::atomic<int> threadsCompleted{ 0 };

    // Lambda function to be executed in threads
    auto threadFunc = [ & logsPerThread, & threadsCompleted](int threadId)
    {
        try
        {
            for(int j = 0; j < logsPerThread; ++j)
            {
                LOG_INFO(logger) << "Thread " << threadId << ", log " << j;
            }
        }
        catch(const std::exception& e)
        {
            std::cerr << "Exception in thread " << threadId << ": " << e.what() << std::endl;
        }
        catch(...)
        {
            std::cerr << "Unknown exception in thread " << threadId << std::endl;
        }
        threadsCompleted++;
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

    // Wait until all logs are written
    if(!logger.waitUntilEmpty(std::chrono::milliseconds(WAIT_UNTIL_EMPTY_MSEC)))
    {
        std::cerr << "Timeout while waiting for task queue to empty" << std::endl;
    }

    //std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Additional delay

    // Check that all logs are written
    LogEntryList allLogs = logger.getAllLogs();

    printLogs(allLogs);

    assert(allLogs.size() == numThreads* logsPerThread);

    std::cout << "testMultiThreadedLogging passed!" << std::endl;
}

void testMultiFilters()
{
    // Clear the database before starting the test
    logger.clearLogs();

    const std::string infoMsg("This is an info message");
    const std::string warningMsg("This is a warning message");
    const std::string errorMsg("This is an error message");

    LOG_INFO(logger) << infoMsg;
    LOG_WARNING(logger) << warningMsg;
    LOG_ERROR(logger) << errorMsg;

    // Wait until all logs are written
    if(!logger.waitUntilEmpty(std::chrono::milliseconds(WAIT_UNTIL_EMPTY_MSEC)))
    {
        std::cerr << "Timeout while waiting for task queue to empty" << std::endl;
    }

    // Define filters
    std::vector<Filter> filters;

    // Filter by log level (only Error messages)
    Filter levelFilter;
    levelFilter.type = Filter::Type::Level;
    levelFilter.field = levelFilter.typeToField();
    levelFilter.value = levelToString(Level::Error); // "ERROR"
    levelFilter.op = "=";
    filters.push_back(levelFilter);

    // Filter by file name (only logs from this file: "test_logger.cpp")
    Filter fileFilter;
    fileFilter.type = Filter::Type::File;
    fileFilter.field = fileFilter.typeToField();
#if ONLY_FILE_NAME == 1
    fileFilter.value = std::filesystem::path(__FILE__).filename().string();
#else
    fileFilter.value = __FILE__;
#endif
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

    // Retrieve logs using the filters
    LogEntryList filteredLogs = logger.getLogsByFilters(filters);

    printLogs(filteredLogs);

    assert(filteredLogs.size() == 1);
    assert(filteredLogs[0].message == errorMsg);

    std::cout << "testMultiFilters passed!" << std::endl;
}

/**
 * @brief Test exporting logs to a file.
 */
void testFileExport()
{
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
    if(!logger.waitUntilEmpty(std::chrono::milliseconds(WAIT_UNTIL_EMPTY_MSEC)))
    {
        std::cerr << "Timeout while waiting for task queue to empty" << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Additional delay

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

    std::cout << "testFileExport passed!" << std::endl;
}

/**
 * @brief Test error handling in the logger.
 */
void testErrorHandling()
{
    // Clear the database before starting the test
    logger.clearLogs();

    //std::unique_ptr<IDatabase> database;

#if TEST_ON_REAL_DB == 0
    // Use MockDatabase
    auto mockDb = std::make_unique<MockDatabase>();
    // Override executeWithParams to simulate an error
    mockDb->executeWithParamsOverride = [](const std::string& query, const std::vector<std::string> & params) -> bool
    {
        return false; // Simulate an error
    };
    database = std::move(mockDb);
#else
    // Use a real SQLite database
    const std::string dbPath = "test_error_handling.db";

    // Remove the old database if it exists
    if(std::filesystem::exists(dbPath))
    {
        std::filesystem::remove(dbPath);
    }

    auto database = std::make_unique<SQLiteDatabase>(dbPath);

    // Simulate an error in the real database
    // For example, try to insert into a non-existent table
    bool success = database->executeWithParams("INSERT INTO non_existent_table (message) VALUES (?)", { "Test message" });
    if(!success)
    {
        // Ensure the error is logged
        std::cerr << "Simulated error in real database: Query failed!" << std::endl;
    }
#endif

    // Create a logger with the selected database
    Logger testLogger(std::move(database));

    // Log a message that will cause an error
    const std::string message = "Error handling test message";
    LOG_INFO(testLogger) << message;

    // Wait until all logs are written
    if(!logger.waitUntilEmpty(std::chrono::milliseconds(WAIT_UNTIL_EMPTY_MSEC)))
    {
        std::cerr << "Timeout while waiting for task queue to empty" << std::endl;
    }

    // Check if the error was logged
    std::ifstream errorLog(std::filesystem::absolute(ERR_LOG_FILE));
    if(!errorLog.is_open())
    {
        std::cerr << "Failed to open error log file:" << std::filesystem::absolute(ERR_LOG_FILE) << std::endl;
        std::cerr << "Error code: " << strerror(errno) << std::endl; // Output system error
        assert(false); // File did not open, test failed
    }

    std::string line;
    bool errorLogged = false;
    while(std::getline(errorLog, line))
    {
        if(line.find(ERR_MSG_FAILED_QUERY) != std::string::npos)
        {
            errorLogged = true;
            break;
        }
    }

    assert(errorLogged);
    std::cout << "testErrorHandling passed!" << std::endl;

    // Cleanup (if a real database was used)
    if(TEST_ON_REAL_DB)
    {
        const std::string dbPath = "test_error_handling.db";
        if(std::filesystem::exists(dbPath))
        {
            std::filesystem::remove(dbPath);
        }
    }
}

/**
 * @brief Remove test files created during testing.
 */
void removeTestFiles()
{
    const auto dbFilePath = std::filesystem::absolute(TEST_DATABASE);

    std::cout << "Remove test files..." << std::endl;

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
}

/**
 * @brief Test the performance of the logger.
 */
void testPerformance()
{
    // Clear the database before starting the test
    logger.clearLogs();

    const int numLogs = PERFORMANCE_TEST_ENTRIES;
    auto startTime = std::chrono::high_resolution_clock::now();

    for(int i = 0; i < numLogs; ++i)
    {
        LOG_INFO(logger) << "Log message " << i;
    }

    // Wait until all logs are written
    if(!logger.waitUntilEmpty(std::chrono::milliseconds(WAIT_UNTIL_EMPTY_MSEC)))
    {
        std::cerr << "Timeout while waiting for task queue to empty" << std::endl;
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    auto stats = logger.getStats();
    std::cout << std::endl << "**** Performance Test ****" << std::endl;
    std::cout << "Logged " << numLogs << " messages in " << duration << " ms" << std::endl;
    std::cout << "Total tasks processed: " << stats.totalTasksProcessed << std::endl;
    std::cout << "Average processing time: " << stats.averageProcessingTime << " ms" << std::endl;
    std::cout << "Max processing time: " << stats.maxProcessingTime << " ms" << std::endl;
}

/**
 * @brief Cleanup function to shut down the logger.
 */
void cleanup()
{
    logger.shutdown();
}

int main()
{
    std::cout << PROJECT_NAME << " v." << VERSION_FULL << std::endl;

#if TEST_ON_REAL_DB == 1
    std::cout << "Test on real database" << std::endl;
    std::cout << TEST_DATABASE << std::endl;
#else
    std::cout << "Test on mock data" << std::endl;
#endif

#if USE_SYNC_MODE == 1
    std::cout << "Sync Mode: ON" << std::endl;
#else
    std::cout << "Sync Mode: OFF" << std::endl;
    std::cout << "Number of threads: " << LOG_NUM_THREADS << std::endl;
#endif

    logger.setLogLevel(LOG_LEVEL);

    try
    {
        testBasicFunctionality();
        testFilterByLevel();
        testFilterByFunction();
        testFilterByThreadId();
        testFilterByFile();
        testFilterByTimestampRange();
        //testErrorHandling(); // TODO:
        //testMultiThread(); // FIXME: SyncMode: OFF
        testMultiFilters();
        testFileExport();
        testClearLogs();
        testPerformance();

        std::cout << "All tests passed successfully!" << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

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