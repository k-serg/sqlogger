# SQLogger

**SQLogger** is a lightweight logging library that uses SQLite as a backend. It provides a simple and flexible way to log messages with different levels (e.g., Info, Warning, Error) and supports both static and dynamic linking.

**Key Features:**

    Simplified Logging: automatically captures the context (function, file, line, and thread ID), making it easier to use.

    Macros: The macros (LOG_INFO, LOG_WARNING, etc.) provide a convenient way to log messages with minimal boilerplate.

    Flexible Log Retrieval: You can retrieve logs by level, timestamp range, file, thread ID, or custom filters.

    Multiple Filters: You can combine multiple filters to narrow down the results. For example, you can filter by level, file, and timestamp range simultaneously.

    Export Functionality: Logs can be exported to various formats (TXT, CSV, XML and JSON) for further analysis.

## Table of Contents

- [Building with CMake](#building-with-cmake)
  - [Requirements](#requirements)
  - [Build Instructions](#build-instructions)
  - [CMake Options](#cmake-options)
  - [Installation](#installation)
- [Public API](#public-api)
  - [Logger Class](#logger-class)
  - [Log Levels](#log-levels)
  - [Macros](#macros)
- [Example Usage](#example-usage)
- [Running Tests](#running-tests)
- [License](#license)

---

## Building with CMake

### Requirements

- CMake version 3.22 or higher
- A C++17-compatible compiler
- SQLite3 (optional, can use the bundled version)

### Build Instructions

1. Clone the repository:

   ```bash
   git clone https://github.com/k-serg/sqlogger.git
   cd sqlogger
   ``` 
    Update submodules:
    ```bash
    git submodule update --init --recursive --remote
    ``` 
    Create a build directory:  
    ```bash
    mkdir build
    cd build
    ``` 
    Run CMake with the desired options:   
    ```bash
    cmake .. [options]
    ```
    Build the project:
    ```bash
    cmake --build .
    ```

### CMake Options

You can customize the build using the following options:

    BUILD_SHARED_LIBS: Build the library as a shared library (default is OFF).
    cmake .. -DBUILD_SHARED_LIBS=ON
    
    USE_SYSTEM_SQLITE: Use the system-installed SQLite library (default is OFF). If OFF, the bundled SQLite will be used.
    cmake .. -DUSE_SYSTEM_SQLITE=ON

    BUILD_TEST: Build the test application (default is ON).
    cmake .. -DBUILD_TEST=OFF

    CMAKE_BUILD_TYPE: Set the build type (e.g., Debug, Release).
    cmake .. -DCMAKE_BUILD_TYPE=Debug

### Installation

After building, you can install the library:
   ```bash
   cmake --install .
   ```
By default, the library will be installed to /usr/local/lib, and the header files will be installed to /usr/local/include.

## Public API
### Logger Class

The Logger class is the main class for logging messages to an SQLite database. It provides methods for logging messages, retrieving logs, and managing log levels.
Constructor

   ```cpp
   Logger(std::unique_ptr<IDatabase> database, bool syncMode = true, size_t numThreads = NUM_THREADS, const bool onlyFileName = false);
   ```

Parameters:

    database: A unique pointer to an IDatabase interface for database operations.

    syncMode: Whether to use synchronous mode (default is true).

    numThreads: The number of threads in the thread pool (default is NUM_THREADS).

    onlyFileName: Log only filename, without full path.

**Method: log**

A simplified method for logging messages. It automatically captures the current context (function name, file name, line number, and thread ID).
   ```cpp
   void log(Level level, const std::string& message);
   ```
Parameters:

    level: The severity level of the log message (e.g., Level::Info, Level::Warning, Level::Error).

    message: The log message.

**Method: logAdd (Private)**

This is the internal method used for logging. It is called by the LogMessage class and should not be used directly.

   ```cpp
   void logAdd(Level level, const std::string& message, const std::string& function, const std::string& file, int line, const std::string& threadId);
   ```

Parameters:

    level: The severity level of the log message.

    message: The log message.

    function: The name of the function where the log was created.

    file: The name of the file where the log was created.

    line: The line number where the log was created.

    threadId: The ID of the thread that created the log.

**Method: getAllLogs**

Retrieves all log entries from the database.

   ```cpp
   LogEntryList getAllLogs();
   ```

Returns:

    A list of all log entries.

**Method: getLogsByLevel**

Retrieves log entries with a specific severity level.

   ```cpp
   LogEntryList getLogsByLevel(const Level& level);
   ```

   Parameters:

    level: The severity level to filter by.

Returns:

    A list of log entries with the specified level.

**Method: getLogsByFilters**

Retrieves log entries based on specified filters. Multiple filters can be combined to narrow down the results.

   ```cpp
   LogEntryList getLogsByFilters(const std::vector<Filter>& filters);
   ```

Parameters:

    filters: A vector of Filter objects representing the filtering criteria.

Returns:

    A list of log entries that match the specified filters.

**Filter Structure**

The Filter structure is used to define filtering criteria. It supports filtering by level, file, function, thread ID, and timestamp range.

   ```cpp
struct Filter
{
    enum class Type
    {
        Unknown = -1,
        Level,
        File,
        Function,
        ThreadId,
        TimestampRange
    };

        Type type; /**< The type to filter on. */
        std::string field; /**< The field to filter on. */
        std::string op;    /**< The operator to use for filtering. */
        std::string value; /**< The value to compare against. */
};
   ```

**Method: setLogLevel**

Sets the minimum log level for messages to be logged.

   ```cpp
    void setLogLevel(Level minLevel);
   ```

Parameters:

    minLevel: The minimum log level (e.g., Level::Warning will log warnings and above).

**Method: shutdown**

Shuts down the logger and stops all worker threads.

   ```cpp
   void shutdown();
   ```

**Method: exportTo**

Exports log entries to a file in a specified format.

   ```cpp
   static void exportTo(const std::string& filePath, const LogExport::Format& format, const LogEntryList& entryList, const std::string& delimiter = ENTRY_DELIMITER, bool name = true);
   ```

Parameters:

    filePath: The path to the output file.

    format: The format of the output file (e.g., LogExport::Format::CSV).

    entryList: The list of log entries to export.

    delimiter: The delimiter to use between fields (default is ENTRY_DELIMITER).

    name: Whether to include field names in the output (default is true).

#### Log Levels

The library supports the following log levels:

    Level::Trace: Trace messages (for detailed debugging).

    Level::Debug: Debug messages (for general debugging).

    Level::Info: Informational messages (for normal operation).

    Level::Warning: Warning messages (for potential issues).

    Level::Error: Error messages (for recoverable errors).

    Level::Fatal: Fatal error messages (for critical failures).

#### Macros

The library provides macros for simplified logging. These macros automatically capture the current context (function name, file name, line number, and thread ID).

    LOG_TRACE(logger): Logs a trace message.

    LOG_DEBUG(logger): Logs a debug message.

    LOG_INFO(logger): Logs an informational message.

    LOG_WARNING(logger): Logs a warning message.

    LOG_ERROR(logger): Logs an error message.

    LOG_FATAL(logger): Logs a fatal error message.

## Example Usage

Here’s an example of how to use the Logger class and its macros:

   ```cpp
#include "logger.h"

int main() {
    // Create a logger instance with a database
    std::unique_ptr<IDatabase> database = std::make_unique<SQLiteDatabase>("logs.db");
    Logger logger(std::move(database)); // logger must own a poiner to the database

    // Set the minimum log level to Warning
    logger.setLogLevel(Level::Warning);

    // Log messages using the simplified log() method
    logger.log(Level::Info, "This is an info message."); // This won't be logged because the level is below Warning
    logger.log(Level::Warning, "This is a warning message.");
    logger.log(Level::Error, "This is an error message.");

    // Log messages using macros
    LOG_INFO(logger) << "This is an info message."; // This won't be logged
    LOG_WARNING(logger) << "This is a warning message.";
    LOG_ERROR(logger) << "This is an error message.";

    // Wait until all logs are written
    if (!logger.waitUntilEmpty())
    {
        std::cerr << "Timeout while waiting for task queue to empty" << std::endl;
    }

    // Retrieve all logs
    LogEntryList allLogs = logger.getAllLogs();
    for (const auto& log : allLogs) {
        std::cout << log.message << std::endl;
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

    // Filter by file name (only logs from this file (N.B. use full path): i.e. "c:\\test_app\\test_logger.cpp")
    Filter fileFilter;
    fileFilter.type = Filter::Type::File;
    fileFilter.field = fileFilter.typeToField();
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

    // Retrieve logs using the filters
    LogEntryList filteredLogs = logger.getLogsByFilters(filters);

    // Print the filtered logs
    for (const auto& log : filteredLogs)
    {
        std::cout << "Level: " << log.level << ", Message: " << log.message
                  << ", File: " << log.file << ", Timestamp: " << log.timestamp << std::endl;
    }

    // Export logs to a CSV file
    Logger::exportTo("logs.csv", LogExport::Format::CSV, allLogs);

    // Shutdown the logger
    logger.shutdown();

    return 0;
}
   ```


## Running tests
If you enabled the BUILD_TEST option, you can run the tests after building the project:

   ```bash
   ./bin/test_sqlogger
   ```

This will execute the test suite and verify that the library is working correctly.

## License

This project is licensed under the GNU GPL V3 License. See the LICENSE file for details.

Copyright (C) 2025  Sergey K.