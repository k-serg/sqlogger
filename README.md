# SQLogger

**SQLogger** is a fast, lightweight, multi-threaded logging library that supports various database backends. It provides a simple and flexible way to log messages with different levels (e.g., Trace, Debug, Info) and includes advanced features like batch processing, statistics collection, and log filtering.

**Key Features:**
- **Multiple Database Backends**: Supports SQLite, MySQL and PostgreSQL
- **Simplified Logging**: Automatically captures context (function, file, line, thread ID)
- **Macros**: Convenient macros (LOG_INFO, LOG_WARNING, etc.) for minimal boilerplate
- **Flexible Log Retrieval**: Filter logs by level, timestamp range, file, thread ID, or custom filters
- **Multiple Filters**: You can combine multiple filters to narrow down the results. For example, you can filter by level, file, and timestamp range simultaneously
- **Batch Processing**: Configurable batch sizes for improved performance
- **Statistics**: Track logging performance metrics
- **Export Functionality**: Export logs to TXT, CSV, XML, JSON and YAML formats
- **Thread-Safe**: Designed for multi-threaded environments
- **Source Tracking**: Optional source identification (applications/services)

## Table of Contents

- [Building with CMake](#building-with-cmake)
  - [Requirements](#requirements)
  - [Build Instructions](#build-instructions)
  - [CMake Options](#cmake-options)
  - [Installation](#installation)
- [Public API](#public-api)
  - [Logger Class](#logger-class)
  - [Configuration Management](#configuration-management)
  - [LogManager](#logmanager)
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
- Database libraries for your chosen backend(s)

### Build Instructions

1. Clone the repository:
   ```bash
   git clone https://github.com/k-serg/sqlogger.git
   cd sqlogger
   ```
   
2. Update submodules:
   ```bash
   git submodule update --init --recursive --remote
   ```
   
3. Create a build directory:  
   ```bash
   mkdir build
   cd build
   ```
   
4. Run CMake with the desired options:   
   ```bash
   cmake .. [options]
   ```
   
5. Build the project:
   ```bash
   cmake --build .
   ```

### CMake Options

Customize the build using these options:

- `BUILD_SHARED_LIBS`: Build as shared library (default ON)
  ```bash
  cmake .. -DBUILD_SHARED_LIBS=OFF
  ```
  
- `USE_SYSTEM_SQLITE`: Use system SQLite 3 instead of bundled SQLite 3 amalgamation (default OFF)
  ```bash
  cmake .. -DUSE_SYSTEM_SQLITE=ON
  ```

- `BUILD_TEST`: Build tests (default ON)
  ```bash
  cmake .. -DBUILD_TEST=OFF
  ```

- `CMAKE_BUILD_TYPE`: Set build type (Debug/Release)
  ```bash
  cmake .. -DCMAKE_BUILD_TYPE=Debug
  ```

- `USE_SOURCE_INFO`: Enable source tracking (default OFF)
  ```bash
  cmake .. -DUSE_SOURCE_INFO=ON
  ```

- `BUILD_DOCS`: Enable html documentation build (default OFF)
  ```bash
  cmake .. -DBUILD_DOCS=ON
  ```
- `USE_AES`: Enable AES config password encryption. By default password encryption use XOR. (default OFF)
  ```bash
  cmake .. -DUSE_AES=ON
  ```
- `USE_MYSQL`: Enable MySQL support (depends on libmysqlclient-dev on Linux and MySQL Server 8.0 on Windows)
  ```bash
  cmake .. -DUSE_MYSQL=ON
  ```
- `USE_POSTGRESQL`: Enable PostgreSQL support (depends on libpq-dev on Linux and PostgreSQL on Windows)
  ```bash
  cmake .. -DUSE_POSTGRESQL=ON
  ```
- 
### Installation

Install the library:
```bash
cmake --install .
```

By default, installs to `/usr/local/lib` and `/usr/local/include`.

## Public API

### Logger Class

The main logging class supporting multiple database backends and advanced features.

#### Key Methods:

**Logging:**
```cpp
// Log a message with specified level (simplified interface)
void log(LogLevel level, const std::string& message);

// Get current logging statistics (counts, timings, batch info)
Stats getStats() const;

// Force immediate write of all buffered log entries
void flush();
```

**Configuration:**
```cpp
// Set minimum log level (messages below this level will be ignored)
void setLogLevel(LogLevel minLevel);

// Configure batch processing size (0 to disable batching)
void setBatchSize(int size);

// Get current logger configuration
LogConfig::Config getConfig() const;
```

**Log Entry Structure:**
```cpp
struct LogEntry
{
    int id;  // Unique log entry ID
#ifdef USE_SOURCE_INFO
    int sourceId;  // Associated source ID if enabled
#endif
    std::string timestamp;  // When the log was created
    std::string level;      // Log level as string
    std::string message;    // The logged message
    std::string function;   // Function where logged
    std::string file;       // Source file where logged
    int line;               // Line number where logged
    std::string threadId;   // Thread ID that created log
#ifdef USE_SOURCE_INFO
    std::string uuid;       // Source UUID if enabled
    std::string sourceName; // Source name if enabled
#endif
};

// List of log entries
using LogEntryList = std::vector<LogEntry>;
```

**Log Entry Methods:**
```cpp
// Format log entry as string with optional field names
std::string print(const std::string& delimiter = ",", bool name = true) const;

// Write log entry to file stream with formatting
void printToFile(std::ofstream& outFile, 
                const std::string& delimiter = ",", 
                bool name = true) const;
```

**Log Retrieval:**
```cpp
// Get all logs
LogEntryList getAllLogs();

// Get logs by severity level
LogEntryList getLogsByLevel(const LogLevel& level,
                           int limit = -1,  // -1 for no limit
                           int offset = -1); // -1 to disable offset

// Get logs within time range (format: "YYYY-MM-DD HH:MM:SS")
LogEntryList getLogsByTimestampRange(const std::string& startTime,
                                    const std::string& endTime,
                                    const int limit = -1,
                                    const int offset = -1);

// Get logs by source file
LogEntryList getLogsByFile(const std::string& file,
                          const int limit = -1,
                          const int offset = -1);

// Get logs by thread ID
LogEntryList getLogsByThreadId(const std::string& threadId,
                              const int limit = -1,
                              const int offset = -1);

// Get logs by function name
LogEntryList getLogsByFunction(const std::string& function,
                              const int limit = -1,
                              const int offset = -1);

// Get logs using custom filters (see Filter structure below)
LogEntryList getLogsByFilters(const std::vector<Filter>& filters,
                             int limit = -1,
                             int offset = -1);

// Source-specific log retrieval (when USE_SOURCE_INFO enabled)
LogEntryList getLogsBySourceId(const int& sourceId,
                              const int limit = -1,
                              const int offset = -1);

LogEntryList getLogsBySourceUuid(const std::string& sourceUuid,
                                const int limit = -1,
                                const int offset = -1);
```

**Filter Structure:**
```cpp
struct Filter
{
    enum class Type
    {
        Unknown = -1,
        Level,         // Filter by log level
        File,          // Filter by source file
        Function,      // Filter by function name
        ThreadId,      // Filter by thread ID
        TimestampRange // Filter by time range
#ifdef USE_SOURCE_INFO
        , SourceId     // Filter by source ID
#endif
    };

    Type type;        // Filter type
    std::string field; // Database field to filter
    std::string op;    // Comparison operator (=, >, <, etc.)
    std::string value; // Value to compare against
};
```

**Source Management (when USE_SOURCE_INFO enabled):**

**Source Info Structure:**
```cpp
struct SourceInfo
{
    int sourceId;     // The unique identifier (ID) of the source
    std::string uuid; // The universally unique identifier (UUID) of the source
    std::string name; // The name of the source
};
```

**Source Info Adding:**
```cpp
// Add new source (returns source ID or SOURCE_NOT_FOUND on failure)
int addSource(const std::string& name, const std::string& uuid);
```

**Source Info Retrieval:**
```cpp
// Get all registered sources
std::vector<SourceInfo> getAllSources();

// Lookup source by ID/UUID/name (returns std::nullopt if not found)
std::optional<SourceInfo> getSourceById(const int sourceId);
std::optional<SourceInfo> getSourceByUuid(const std::string& uuid);
std::optional<SourceInfo> getSourceByName(const std::string& name);
```

**Export:**
```cpp
// Export logs to file in specified format
static void exportTo(const std::string& filePath, 
                    const LogExport::Format& format,
                    const LogEntryList& entryList, 
                    const std::string& delimiter = ",", 
                    bool name = true);

// Supported export formats
enum class Format
{
    TXT,
    CSV,
    XML,
    JSON,
    YAML
};
```

### Configuration Management

The `LogConfig::Config` structure handles logger configuration:

```cpp
struct Config
{
    // Core logger settings
    std::optional<std::string> name;          // Logger instance name
    std::optional<bool> syncMode;             // Use synchronous logging
    std::optional<size_t> numThreads;         // Worker threads count
    std::optional<bool> onlyFileNames;        // Store only filenames (no paths)
    std::optional<LogLevel> minLogLevel;      // Minimum log level
    std::optional<bool> useBatch;             // Enable batch processing
    std::optional<int> batchSize;             // Max batch size
    
    // Database connection settings
    std::optional<std::string> databaseName;  // Database name/path
    std::optional<std::string> databaseTable; // Table name
    std::optional<std::string> databaseHost;  // Database host
    std::optional<int> databasePort;          // Connection port
    std::optional<std::string> databaseUser;  // Username
    std::optional<std::string> databasePass;  // Password
    std::optional<DataBaseType> databaseType; // Database backend type
    
    // Source tracking (when USE_SOURCE_INFO enabled)
#ifdef USE_SOURCE_INFO
    std::optional<std::string> sourceUuid;    // Source UUID
    std::optional<std::string> sourceName;    // Source name
#endif
    
    // Security
    std::optional<std::string> passKey;       // Encryption key for passwords
    
    // Load configuration from INI file
    static Config loadFromINI(const std::string& filename = "sqlogger.ini", 
                            const std::string& passKey = "");
    
    // Save configuration to INI file
    static void saveToINI(const Config& config, 
                        const std::string& filename = "sqlogger.ini");
    
    // Encrypt/decrypt sensitive data
    void setPassKey(const std::string& passKey);
    std::string getPassKey();
};
```

**INI File Configuration:**
The library supports loading/saving configuration from INI files with these sections:

```
[Logger]
Name = MyLogger
SyncMode = false
NumThreads = 4
OnlyFileNames = true
MinLogLevel = Info
UseBatch = true
BatchSize = 100

[Database]
Type = SQLite
Database = logs.db
Table = logs
# For networked databases:
# Host = localhost
# Port = 3306
# User = root
# Pass = encrypted_password

[Source]  # When USE_SOURCE_INFO enabled
Uuid = 550e8400-e29b-41d4-a716-446655440000
Name = MyApplication
```

### LogManager

Singleton class for centralized logger management:

```cpp
// Get singleton instance
static LogManager& getInstance();

// Create/retrieve loggers
Logger& createLogger(const std::string& name, const LogConfig::Config& config);
Logger& getLogger(const std::string& name);

// Management
void removeLogger(const std::string& name);
int removeAllLoggers();
template<typename Predicate> int removeIf(Predicate&& predicate);
```

### Log Levels

Supported log levels:
- `LogLevel::Trace`
- `LogLevel::Debug` 
- `LogLevel::Info`
- `LogLevel::Warning`
- `LogLevel::Error`
- `LogLevel::Fatal`

### Macros

Simplified logging macros:
```cpp
LOG_TRACE(logger) << "Message";
LOG_DEBUG(logger) << "Message"; 
LOG_INFO(logger) << "Message";
LOG_WARNING(logger) << "Message";
LOG_ERROR(logger) << "Message";
LOG_FATAL(logger) << "Message";
```

## Example Usage

```cpp
#include "log_manager.h"

int main() {
    // Configure logger
    LogConfig::Config config;
    config.name = "main"
    config.databaseType = DataBaseType::SQLite;
    config.databaseName = "logs.db";
    config.minLogLevel = LogLevel::Info;
    config.syncMode = false;
    config.numThreads = 4;
    config.useBatch = true;
    config.batchSize = 100;

    // Create logger via LogManager
    auto& logger = LogManager::getInstance().createLogger("main", config);

    // Log messages
    LOG_INFO(logger) << "Application started";
    LOG_WARNING(logger) << "Low disk space";
    
    // Get statistics
    auto stats = logger.getStats();
    std::cout << Logger::getFormattedStats(stats);

    // Retrieve logs
    auto allLogs = logger.getAllLogs();
    auto errorLogs = logger.getLogsByLevel(LogLevel::Error);

    // Export to CSV
    Logger::exportTo("logs.csv", LogExport::Format::CSV, allLogs);

    // Clean up
    LogManager::getInstance().removeLogger("main");
    return 0;
}
```

## Running Tests

If tests are enabled:
```bash
./bin/test_sqlogger
```

## License

This project is licensed under the GNU GPL V3 License. See the LICENSE file for details.

Copyright (C) 2025 Sergey K.
