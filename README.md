# SQLogger

**SQLogger** is a lightweight, multi-threaded logging library that supports various database backends. It provides a simple and flexible way to log messages with different levels (e.g., Trace, Debug, Info) and includes advanced features like batch processing, statistics collection, and log filtering.

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
void log(LogLevel level, const std::string& message);
Stats getStats() const;
void flush();
```

**Configuration:**
```cpp
void setLogLevel(LogLevel minLevel);
void setBatchSize(int size);
LogConfig::Config getConfig() const;
```

**Log Entry Structure:**
```cpp
struct LogEntry
{
    int id;
#ifdef USE_SOURCE_INFO
    int sourceId;
#endif
    std::string timestamp;
    std::string level;
    std::string message;
    std::string function;
    std::string file;
    int line;
    std::string threadId;
#ifdef USE_SOURCE_INFO
    std::string uuid;
    std::string sourceName;
#endif

LogEntryList = std::vector<LogEntry>;
```

**Log Entry Methods:**
```cpp
std::string print(const std::string& delimiter = ENTRY_DELIMITER,
                                       bool name = true) const;

void printToFile(std::ofstream& outFile,
                                       const std::string& delimiter = ENTRY_DELIMITER,
                                       bool name = true) const;
```

**Log Retrieval:**
```cpp
LogEntryList getAllLogs();

LogEntryList getLogsByLevel(const LogLevel& level,
                                       int limit = -1,
                                       int offset = -1);

LogEntryList getLogsByTimestampRange(const std::string& startTime,
                                       const std::string& endTime,
                                       const int limit = -1,
                                       const int offset = -1);

LogEntryList getLogsByFile(const std::string& file,
                                       const int limit = -1,
                                       const int offset = -1);

LogEntryList getLogsByThreadId(const std::string& threadId,
                                       const int limit = -1,
                                       const int offset = -1);

LogEntryList getLogsByFunction(const std::string& function,
                                       const int limit = -1,
                                       const int offset = -1);


LogEntryList getLogsByFilters(const std::vector<Filter>& filters,
                                       int limit = -1,
                                       int offset = -1);

LogEntryList getLogsBySourceId(const int& sourceId,
                               const int limit = -1,
                               const int offset = -1);

LogEntryList getLogsBySourceUuid(const std::string& sourceUuid,
                                 const int limit = -1,
                                 const int offset = -1);

int addSource(const std::string& name, const std::string& uuid);

std::vector<SourceInfo> getAllSources();

std::optional<SourceInfo> getSourceById(const int sourceId);

std::optional<SourceInfo> getSourceByUuid(const std::string& uuid);

std::optional<SourceInfo> getSourceByName(const std::string& name);
```

**Source Info Structure:**
```cpp
struct SourceInfo
{
    int sourceId;
    std::string uuid;
    std::string name;
};
```

**Source Info Adding:**
```cpp
int addSource(const std::string& name, const std::string& uuid);
```

**Source Info Retrieval:**
```cpp
int addSource(const std::string& name, const std::string& uuid);

std::vector<SourceInfo> getAllSources();

std::optional<SourceInfo> getSourceById(const int sourceId);

std::optional<SourceInfo> getSourceByUuid(const std::string& uuid);

std::optional<SourceInfo> getSourceByName(const std::string& name);
```

**Export:**
```cpp
static void exportTo(const std::string& filePath, const LogExport::Format& format, 
                   const LogEntryList& entryList, const std::string& delimiter = ",", 
                   bool name = true);

enum class Format
    {
        TXT,
        CSV,
        XML,
        JSON,
        YAML
    };
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
#include "logger.h"
#include "log_manager.h"

int main() {
    // Configure logger
    LogConfig::Config config;
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
