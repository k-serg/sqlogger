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

#include "log_config.h"
#include "ini_parser.h"

/**
* @brief Joins a vector of strings into a single string with a delimiter
* @param parts The vector of strings to join
* @param delimiter The string to insert between joined parts
* @return std::string The resulting concatenated string
*/
std::string StringHelper::join(const std::vector<std::string> & parts, const std::string& delimiter)
{
    std::string result;
    for(size_t i = 0; i < parts.size(); ++i)
    {
        if(i != 0)
        {
            result += delimiter;
        }
        result += parts[i];
    }
    return result;
};

/**
* @brief Splits a string into a vector of strings using a delimiter
* @param str The string to split
* @param delimiter The string to use as delimiter
* @return std::vector<std::string> The resulting vector of strings
*/
std::vector<std::string> StringHelper::split(const std::string& str, const std::string& delimiter)
{
    std::vector<std::string> result;
    size_t start = 0;
    size_t end = str.find(delimiter);

    while(end != std::string::npos)
    {
        result.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
        end = str.find(delimiter, start);
    }

    result.push_back(str.substr(start));

    return result;
};

namespace LogConfig
{
    /**
        * @brief Checks if validation was successful.
        * @return true if all validations passed (no missing or invalid parameters).
        * @return false if any validation failed.
        */
    inline bool ValidateResult::ok() const
    {
        return success && missingParams.empty() && invalidParams.empty();
    };

    /**
    * @brief Formats validation errors as human-readable string.
    * @return std::string Formatted error message containing:
    * - Missing parameters (comma-separated).
    * - Invalid parameters with details (one per line).
    */
    std::string ValidateResult::print() const
    {
        std::stringstream ss;

        if(!missingParams.empty())
        {
            ss << "Missing params: " << std::endl;
            ss << StringHelper::join(missingParams, ", ");
            ss << std::endl;
        }

        if(!invalidParams.empty())
        {
            ss << "Invalid params:\n";
            for(const auto & [param, detail] : invalidParams)
            {
                ss << param << ": " << detail << std::endl;
            }
        }
        ss << std::endl;
        return ss.str();
    };

    /**
    * @brief Records a missing parameter error.
    * @param param Name of the missing parameter.
    * @note Automatically sets success=false.
    */
    inline void ValidateResult::addMissing(const::std::string& param)
    {
        success = false;
        missingParams.emplace_back(param);
    };

    /**
    * @brief Records an invalid parameter error.
    * @param param Name of the invalid parameter.
    * @param details Description of the validation failure.
    * @note Automatically sets success=false.
    */
    inline void ValidateResult::addInvalid(const::std::string& param, const::std::string& details)
    {
        success = false;
        invalidParams.emplace_back(param, details);
    };

    /**
        * @brief Combines validation results from another ValidateResult.
        * @param other Another validation result to merge into this one.
        * @note Merged result will be unsuccessful if either source was unsuccessful.
        */
    void ValidateResult::merge(const ValidateResult& other)
    {
        if(!other.success)
        {
            success = false;
        }
        missingParams.insert(missingParams.end(), other.missingParams.begin(), other.missingParams.end());
        invalidParams.insert(invalidParams.end(), other.invalidParams.begin(), other.invalidParams.end());
    };

    /**
    * @brief Sets the password key for password encryption and decryption.
    * @param passKey The secret key for password encryption and decryption.
    * @throws std::runtime_error If passKey empty.
    * @note The key is stored in memory in plaintext. Consider additional security
    * measures if handling highly sensitive data.
    * @warning Avoid hardcoding keys in source code. Prefer secure configuration
    * or key management systems for production environments.
    */
    void Config::setPassKey(const std::string& passKey)
    {
        if(!passKey.empty())
            this->passKey = passKey;
        else
            throw std::runtime_error(ERR_MSG_PASSKEY_EMPTY);
    };

    /**
    * @brief Retrieves the current password key
    * @return std::string The stored password key
    * @throws std::runtime_error If no key has been set (when passKey is empty)
    * @note The returned key should be handled securely and cleared from memory
    * when no longer needed.
    * @warning Exposing the key through logs or debugging outputs creates
    * security vulnerabilities.
    */
    std::string Config::getPassKey()
    {
        if(passKey.has_value())
            return passKey.value();
        else
            throw std::runtime_error(ERR_MSG_PASSKEY_EMPTY);
    };

    /**
     * @brief Loads configuration from an INI file.
     * @param filename The path to the INI file.
     * @param passKey The key for password decryption.
     * @return A Config object containing the loaded settings.
     * @throws std::runtime_error If the file cannot be parsed.
     */
    Config Config::loadFromINI(const std::string& filename, const std::string& passKey)
    {
        auto iniData = INI::parse(filename);
        Config config;

        if(!passKey.empty())
        {
            config.passKey = passKey;
        }

        if(iniData.count(LOG_INI_SECTION_LOGGER))
        {
            const auto& loggerSection = iniData[LOG_INI_SECTION_LOGGER];
            if(loggerSection.count(LOG_INI_KEY_NAME))
            {
                config.name = loggerSection.at(LOG_INI_KEY_NAME);
            }
            if(loggerSection.count(LOG_INI_KEY_SYNC_MODE))
            {
                config.syncMode = LogHelper::toLowerCase(loggerSection.at(LOG_INI_KEY_SYNC_MODE)) == "true";
            }
            if(loggerSection.count(LOG_INI_KEY_NUM_THREADS))
            {
                if(LogHelper::isNumeric(loggerSection.at(LOG_INI_KEY_NUM_THREADS)))
                {
                    config.numThreads = std::stoul(loggerSection.at(LOG_INI_KEY_NUM_THREADS));
                }
                else
                {
                    config.numThreads = std::nullopt;
                }
            }
            if(loggerSection.count(LOG_INI_KEY_ONLY_FILE_NAMES))
            {
                config.onlyFileNames = LogHelper::toLowerCase(loggerSection.at(LOG_INI_KEY_ONLY_FILE_NAMES)) == "true";
            }
            if(loggerSection.count(LOG_INI_KEY_MIN_LOG_LEVEL))
            {
                if(LogHelper::stringToLevel(loggerSection.at(LOG_INI_KEY_MIN_LOG_LEVEL)) != LogLevel::Unknown)
                {
                    config.minLogLevel = LogHelper::stringToLevel(loggerSection.at(LOG_INI_KEY_MIN_LOG_LEVEL));
                }
                else
                {
                    config.minLogLevel = std::nullopt;
                }
            }
            if(loggerSection.count(LOG_INI_KEY_USE_BATCH))
            {
                config.useBatch = LogHelper::toLowerCase(loggerSection.at(LOG_INI_KEY_USE_BATCH)) == "true";
            }
            if(loggerSection.count(LOG_INI_KEY_BATCH_SIZE))
            {
                if(LogHelper::isNumeric(loggerSection.at(LOG_INI_KEY_BATCH_SIZE)))
                {
                    config.batchSize = std::stoi(loggerSection.at(LOG_INI_KEY_BATCH_SIZE));
                }
                else
                {
                    config.batchSize = std::nullopt;
                }
            }
        }
        if(iniData.count(LOG_INI_SECTION_DATABASE))
        {
            const auto& databaseSection = iniData[LOG_INI_SECTION_DATABASE];
            if(databaseSection.count(LOG_INI_KEY_DATABASE_NAME))
            {
                config.databaseName = databaseSection.at(LOG_INI_KEY_DATABASE_NAME);
            }
            if(databaseSection.count(LOG_INI_KEY_DATABASE_TABLE))
            {
                config.databaseTable = databaseSection.at(LOG_INI_KEY_DATABASE_TABLE);
            }
            if(databaseSection.count(LOG_INI_KEY_DATABASE_HOST))
            {
                config.databaseHost = databaseSection.at(LOG_INI_KEY_DATABASE_HOST);
            }
            if(databaseSection.count(LOG_INI_KEY_DATABASE_PORT))
            {
                if(LogHelper::isNumeric(databaseSection.at(LOG_INI_KEY_DATABASE_PORT)))
                {
                    config.databasePort = std::stoi(databaseSection.at(LOG_INI_KEY_DATABASE_PORT));
                }
                else
                {
                    config.databasePort = std::nullopt;
                }
            }
            if(databaseSection.count(LOG_INI_KEY_DATABASE_USER))
            {
                config.databaseUser = databaseSection.at(LOG_INI_KEY_DATABASE_USER);
            }
            if(databaseSection.count(LOG_INI_KEY_DATABASE_PASS))
            {
                if(!config.passKey.has_value() || config.passKey.value().empty())
                {
                    throw std::runtime_error(ERR_MSG_PASSKEY_EMPTY);
                }
                else
                {
                    config.databasePass = LogCrypto::decrypt(databaseSection.at(LOG_INI_KEY_DATABASE_PASS), config.passKey.value());
                }
            }
            if(databaseSection.count(LOG_INI_KEY_DATABASE_TYPE))
            {
                try
                {
                    config.databaseType = DataBaseHelper::stringToDatabaseType(databaseSection.at(LOG_INI_KEY_DATABASE_TYPE));
                }
                catch(...)
                {
                    config.databaseType = std::nullopt;
                }
            }
        }
#ifdef USE_SOURCE_INFO
        if(iniData.count(LOG_INI_SECTION_SOURCE))
        {
            const auto& databaseSection = iniData[LOG_INI_SECTION_SOURCE];
            if(databaseSection.count(LOG_INI_KEY_SOURCE_UUID))
            {
                config.sourceUuid = databaseSection.at(LOG_INI_KEY_SOURCE_UUID);
            }
            if(databaseSection.count(LOG_INI_KEY_SOURCE_NAME))
            {
                config.sourceName = databaseSection.at(LOG_INI_KEY_SOURCE_NAME);
            }
        }
#endif
        return config;
    };

    /**
     * @brief Saves configuration to an INI file.
     * @param config The Config object to save.
     * @param filename The path to the INI file.
     * @throws std::runtime_error If the file cannot be written.
     */
    void Config::saveToINI(const Config& config, const std::string& filename)
    {
        INI::INIData iniData;

        if(config.name.has_value())
        {
            iniData[LOG_INI_SECTION_LOGGER][LOG_INI_KEY_NAME] = config.name.value();
        }
        if(config.syncMode.has_value())
        {
            iniData[LOG_INI_SECTION_LOGGER][LOG_INI_KEY_SYNC_MODE] = config.syncMode.value() ? "true" : "false";
        }
        if(config.numThreads.has_value())
        {
            iniData[LOG_INI_SECTION_LOGGER][LOG_INI_KEY_NUM_THREADS] = std::to_string(config.numThreads.value());
        }
        if(config.onlyFileNames.has_value())
        {
            iniData[LOG_INI_SECTION_LOGGER][LOG_INI_KEY_ONLY_FILE_NAMES] = config.onlyFileNames.value() ? "true" : "false";
        }
        if(config.minLogLevel.has_value())
        {
            iniData[LOG_INI_SECTION_LOGGER][LOG_INI_KEY_MIN_LOG_LEVEL] = LogHelper::levelToString(config.minLogLevel.value());
        }
        if(config.useBatch.has_value())
        {
            iniData[LOG_INI_SECTION_LOGGER][LOG_INI_KEY_USE_BATCH] = config.useBatch.value() ? "true" : "false";
        }
        if(config.batchSize.has_value())
        {
            iniData[LOG_INI_SECTION_LOGGER][LOG_INI_KEY_BATCH_SIZE] = std::to_string(config.batchSize.value());
        }
        if(config.databaseName.has_value())
        {
            iniData[LOG_INI_SECTION_DATABASE][LOG_INI_KEY_DATABASE_NAME] = config.databaseName.value();
        }
        if(config.databaseTable.has_value())
        {
            iniData[LOG_INI_SECTION_DATABASE][LOG_INI_KEY_DATABASE_TABLE] = config.databaseTable.value();
        }
        if(config.databaseHost.has_value())
        {
            iniData[LOG_INI_SECTION_DATABASE][LOG_INI_KEY_DATABASE_HOST] = config.databaseHost.value();
        }
        if(config.databasePort.has_value())
        {
            iniData[LOG_INI_SECTION_DATABASE][LOG_INI_KEY_DATABASE_PORT] = std::to_string(config.databasePort.value());
        }
        if(config.databaseUser.has_value())
        {
            iniData[LOG_INI_SECTION_DATABASE][LOG_INI_KEY_DATABASE_USER] = config.databaseUser.value();
        }
        if(config.databasePass.has_value())
        {
            if(!config.passKey.has_value() || config.passKey.value().empty())
            {
                throw(ERR_MSG_PASSKEY_EMPTY);
            }
            else
            {
                iniData[LOG_INI_SECTION_DATABASE][LOG_INI_KEY_DATABASE_PASS] = LogCrypto::encrypt(config.databasePass.value(), config.passKey.value());
            }
        }
        if(config.databaseType.has_value())
        {
            iniData[LOG_INI_SECTION_DATABASE][LOG_INI_KEY_DATABASE_TYPE] = DataBaseHelper::databaseTypeToString(config.databaseType.value());
        }
#ifdef USE_SOURCE_INFO
        if(config.sourceUuid.has_value())
        {
            iniData[LOG_INI_SECTION_SOURCE][LOG_INI_KEY_SOURCE_UUID] = config.sourceUuid.value();
        }
        if(config.sourceName.has_value())
        {
            iniData[LOG_INI_SECTION_SOURCE][LOG_INI_KEY_SOURCE_NAME] = config.sourceName.value();
        }
#endif
        INI::write(filename, iniData);
    };

    /**
    * @brief Converts a LogConfig::Config object into a database-specific connection string
    * @param config Configuration object containing database connection parameters
    * @return std::string Formatted connection string appropriate for the specified database type
    *
    * @throws std::runtime_error If:
    *         - Database type is not specified in config
    *         - Unsupported database type is requested
    *
    * @note Connection string format varies by database type:
    *       - Mock/SQLite: Returns just the database name (or empty string)
    *       - MySQL: "host=value;user=value;password=value" format
    *       - PostgreSQL: "host=value user=value password=value" format (space-separated)
    *       - MongoDB: "host=value;user=value;password=value" format
    *
    * @warning All parameters are optional except databaseType. Missing parameters will be omitted
    *          from the connection string which may cause connection failures.
    *
    * @see LogConfig::Config
    * @see DataBaseType
    * @see StringHelper::join()
    */
    std::string configToConnectionString(const Config& config)
    {
        if(!config.databaseType.has_value())
        {
            throw std::runtime_error("Database type is not specified in config");
        }

        switch(config.databaseType.value())
        {
            case DataBaseType::Mock:
                return config.databaseName.value_or("");
                break;

            case DataBaseType::SQLite:
                return config.databaseName.value_or("");
                break;

            case DataBaseType::MySQL:
            {
                std::vector<std::string> parts;
                if(config.databaseHost.has_value())
                    parts.emplace_back(std::string(CON_STR_HOST) + "=" + config.databaseHost.value());
                if(config.databaseUser.has_value())
                    parts.emplace_back(std::string(CON_STR_USER) + "=" + config.databaseUser.value());
                if(config.databasePass.has_value())
                    parts.emplace_back(std::string(CON_STR_PASS) + "=" + config.databasePass.value());
                if(config.databaseName.has_value())
                    parts.emplace_back(std::string(CON_STR_DB) + "=" + config.databaseName.value());
                if(config.databasePort.has_value())
                    parts.emplace_back(std::string(CON_STR_PORT) + "=" + std::to_string(config.databasePort.value()));

                return StringHelper::join(parts, ";");
            }
            break;

            case DataBaseType::PostgreSQL:
            {
                std::vector<std::string> parts;
                if(config.databaseHost.has_value())
                    parts.emplace_back("host=" + config.databaseHost.value());
                if(config.databaseUser.has_value())
                    parts.emplace_back("user=" + config.databaseUser.value());
                if(config.databasePass.has_value())
                    parts.emplace_back("password=" + config.databasePass.value());
                if(config.databaseName.has_value())
                    parts.emplace_back("dbname=" + config.databaseName.value());
                if(config.databasePort.has_value())
                    parts.emplace_back("port=" + std::to_string(config.databasePort.value()));

                return StringHelper::join(parts, " ");
            }
            break;

            case DataBaseType::MongoDB:
            {
                //std::vector<std::string> parts;
                //if(config.databaseHost.has_value())
                //    parts.emplace_back("host=" + config.databaseHost.value());
                //if(config.databaseUser.has_value())
                //    parts.emplace_back("user=" + config.databaseUser.value());
                //if(config.databasePass.has_value())
                //    parts.emplace_back("password=" + config.databasePass.value());
                //if(config.databaseName.has_value())
                //    parts.emplace_back("dbname=" + config.databaseName.value());
                //if(config.databasePort.has_value())
                //    parts.emplace_back("port=" + std::to_string(config.databasePort.value()));

                //return StringHelper::join(parts, ";");
                std::string uri = "mongodb://";

                if(config.databaseUser.has_value() && config.databasePass.has_value())
                {
                    uri += config.databaseUser.value() + ":" + config.databasePass.value() + "@";
                }

                uri += config.databaseHost.value_or("localhost");

                if(config.databasePort.has_value())
                {
                    uri += ":" + std::to_string(config.databasePort.value());
                }

                uri += "/" + config.databaseName.value_or("test");

                return uri;
            }
            break;

            default:
                throw std::runtime_error(ERR_MSG_UNSUPPORTED_DB);
        }
    };

    /**
    * @brief Checks for potential SQL injection patterns in input string
    * @param input The string to validate
    * @return true if dangerous patterns detected, false otherwise
    * @note Uses case-insensitive comparison via LogHelper::toLowerCase()
    */
    bool Config::containsSQLInjection(const std::string& input) const
    {
        const std::string lowerInput = LogHelper::toLowerCase(input);

        for(const auto & pattern : dangerousSQLPatterns)
        {
            if(lowerInput.find(pattern) != std::string::npos)
            {
                return true;
            }
        }
        return false;
    };

    /**
    * @brief Validates the entire configuration
    * @return ValidateResult Contains validation status and error details:
    * - success: true if all validations pass
    * - missingParams: List of missing required parameters
    * - invalidParams: List of invalid parameters with error messages
    * @note This method combines results from all specific validators (name, database, etc.)
    * @see validateName(), validateDatabase(), validateThreads()
    * @see validateSource(), validateBatch(), validateLogLevel()
    */
    ValidateResult Config::validate() const
    {
        ValidateResult finalResult;

        ValidateResult nameResult = validateName();
        if(!nameResult.ok())
        {
            finalResult.merge(nameResult);
        }

        ValidateResult levelResult = validateLogLevel();
        if(!levelResult.ok())
        {
            finalResult.merge(levelResult);
        }

        ValidateResult threadsResult = validateThreads();
        if(!threadsResult.ok())
        {
            finalResult.merge(threadsResult);
        }

        ValidateResult batchResult = validateBatch();
        if(!batchResult.ok())
        {
            finalResult.merge(batchResult);
        }

        ValidateResult databaseResult = validateDatabase();
        if(!databaseResult.ok())
        {
            finalResult.merge(databaseResult);
        }

#ifdef USE_SOURCE_INFO
        ValidateResult uuidResult = validateSource();
        if(!uuidResult.ok())
        {
            finalResult.merge(uuidResult);
        }
#endif
        return finalResult;
    };

    /**
    * @brief Validates the entire configuration (static method)
    * @return ValidateResult Contains validation status and error details:
    * - success: true if all validations pass
    * - missingParams: List of missing required parameters
    * - invalidParams: List of invalid parameters with error messages
    * @note This method combines results from all specific validators (name, database, etc.)
    * @see validateName(), validateDatabase(), validateThreads()
    * @see validateSource(), validateBatch(), validateLogLevel()
    */
    ValidateResult Config::validate(const Config& config)
    {
        return config.validate();
    };

    /**
    * @brief Validates the logger name configuration
    * @return ValidateResult Contains:
    * - success: true if name is valid
    * - missingParams: Contains "name" if empty/missing
    * - invalidParams: Empty (no invalid state possible for name)
    * @details Checks:
    * - Name is not empty (if present)
    * - Name meets length requirements (if any)
    */
    ValidateResult Config::validateName() const
    {
        ValidateResult result;
        if(name && name->empty() || !name)
        {
            result.addMissing(tagLogger + std::string(LOG_INI_KEY_NAME));
        }
        return result;
    };

    /**
    * @brief Validates database connection parameters
    * @return ValidateResult Contains:
    * - success: true if all database parameters are valid
    * - missingParams: List of missing required database params
    * - invalidParams: List of invalid database params with error details
    * @details Checks:
    * - Database type is supported
    * - Required parameters for the database type are present
    * - Port number is valid (0-65535)
    * - Credentials are provided (if required)
    * - Table name is specified
    */
    ValidateResult Config::validateDatabase() const
    {
        ValidateResult result;

        if(!databaseType || !databaseName || !databaseTable
                || databaseName->empty() || databaseTable->empty())
        {
            if(!databaseType)
                result.addMissing(tagDatabase + std::string(LOG_INI_KEY_DATABASE_TYPE));

            if(!databaseName || databaseName->empty())
                result.addMissing(tagDatabase + std::string(LOG_INI_KEY_DATABASE_NAME));

            if(!databaseTable || databaseTable->empty())
                result.addMissing(tagDatabase + std::string(LOG_INI_KEY_DATABASE_TABLE));
        }

        if(databaseType && DataBaseHelper::isDataBaseServer( * databaseType)
                && (!databaseHost || !databaseUser
                    || !databasePass || !databasePort)
          )
        {
            if(!databaseHost || databaseHost->empty())
                result.addMissing(tagDatabase + std::string(LOG_INI_KEY_DATABASE_HOST));

            if(!databaseUser || databaseUser->empty())
                result.addMissing(tagDatabase + std::string(LOG_INI_KEY_DATABASE_USER));

            if(!databasePass || databasePass->empty())
                result.addMissing(tagDatabase + std::string(LOG_INI_KEY_DATABASE_PASS));

            if(!databasePort)
                result.addMissing(tagDatabase + std::string(LOG_INI_KEY_DATABASE_PORT));

            if(databasePort && * databasePort > LOG_MAX_PORT_NUM)
                result.addInvalid(tagDatabase + std::string(LOG_INI_KEY_DATABASE_PORT),
                                  std::string("Port number bigger than ") + std::to_string(LOG_MAX_PORT_NUM));
            if(databasePort && * databasePort < LOG_MIN_PORT_NUM)
                result.addInvalid(tagDatabase + std::string(LOG_INI_KEY_DATABASE_PORT),
                                  std::string("Port number lesser than ") + std::to_string(LOG_MIN_PORT_NUM));
        }

        if(databaseType && !DataBaseHelper::isDataBaseSupported( * databaseType))
        {
            result.addInvalid(tagDatabase + std::string(LOG_INI_KEY_DATABASE_TYPE), "Requested database type "
                              + DataBaseHelper::databaseTypeToString( * databaseType)
                              + " not supported in this build");
        }

        auto validateSQLInjection = [ & ](const std::string& keyName, const std::optional<std::string> & value)
        {
            if(value && containsSQLInjection(value.value()))
            {
                result.addInvalid(tagDatabase + keyName, "Contains dangerous SQL pattern '" + value.value() + "'");
            }
        };

        validateSQLInjection(LOG_INI_KEY_DATABASE_NAME, databaseName);
        validateSQLInjection(LOG_INI_KEY_DATABASE_TABLE, databaseTable);
        validateSQLInjection(LOG_INI_KEY_DATABASE_USER, databaseUser);

        return result;
    };

    /**
    * @brief Validates threading configuration
    * @return ValidateResult Contains:
    * - success: true if thread configuration is valid
    * - missingParams: Empty (thread count has default value)
    * - invalidParams: Contains error if thread count is out of valid range
    * @details Checks:
    * - Thread count is within allowed range (1-256)
    * - Thread count is present if async mode is enabled
    */
    ValidateResult Config::validateThreads() const
    {
        ValidateResult result;

        if(!syncMode)
        {
            result.addMissing(tagLogger + std::string(LOG_INI_KEY_SYNC_MODE));
        }
        else
        {
            if(!syncMode.value() && numThreads
                    && * numThreads < LOG_NUM_THREADS_MIN
                    || * numThreads > LOG_NUM_THREADS_MAX)
            {
                std::ostringstream detail;
                detail << "Threads count could not be "
                       << (numThreads < 1
                           ? "lesser than " + std::to_string(LOG_NUM_THREADS_MIN)
                           : "bigger than " + std::to_string(LOG_NUM_THREADS_MAX))
                       << " (" << * numThreads << ")";
                result.addInvalid(tagLogger + std::string(LOG_INI_KEY_NUM_THREADS), detail.str());
            }
        }
        return result;
    };

    /**
    * @brief Validates batch configuration
    * @return ValidateResult Contains:
    * - success: true if batch configuration is valid
    * - missingParams: Empty (batch size has default value)
    * - invalidParams: Contains error if batch size is out of valid range
    * @details Checks:
    * - Batch size is within allowed range for database type (1-10000 depends on database type)
    * - Batch size is present if async mode is enabled
    * @see getMaxBatchSize()
    * @see DB_MAX_BATCH_SQLITE, DB_MAX_BATCH_MYSQL, DB_MAX_BATCH_POSTGRESQL
    */
    ValidateResult Config::validateBatch() const
    {
        ValidateResult result;

        if(!useBatch)
        {
            result.addMissing(tagLogger + std::string(LOG_INI_KEY_USE_BATCH));
        }

        if(useBatch && useBatch.value())
        {
            if(!batchSize)
            {
                result.addMissing(tagLogger + std::string(LOG_INI_KEY_BATCH_SIZE));
            }
            else
            {
                if(databaseType)
                {
                    if( * batchSize < 1 || * batchSize > DataBaseHelper::getMaxBatchSize( * databaseType))
                    {
                        std::ostringstream detail;
                        detail << "Batch size for "
                               << DataBaseHelper::databaseTypeToString( * databaseType)
                               << " could not be "
                               << (batchSize < DB_MIN_BATCH_SIZE
                                   ? "lesser than " + std::to_string(DB_MIN_BATCH_SIZE)
                                   : "bigger than " + std::to_string(DataBaseHelper::getMaxBatchSize( * databaseType)))
                               << " (" << * batchSize << ")";
                        result.addInvalid(tagLogger + std::string(LOG_INI_KEY_BATCH_SIZE), detail.str());
                    }
                }
            }
        }
        return result;
    };

    /**
    * @brief Validates the minimum log level configuration
    * @return ValidateResult Contains validation status with:
    * - success: true if log level is properly configured
    * - missingParams: Contains log level key if not specified
    * - invalidParams: Empty (no invalid state possible for log level)
    * @details Checks:
    * - Minimum log level is specified in configuration
    * - Log level value is within allowed range (implied by LogLevel enum)
    * @note The actual validity of the log level value is ensured by
    * LogHelper::stringToLevel() during configuration loading.
    * This method only verifies presence of the setting.
    * @see LOG_INI_KEY_MIN_LOG_LEVEL
    * @see LogLevel
    * @see LogHelper::stringToLevel()
    */
    ValidateResult Config::validateLogLevel() const
    {
        ValidateResult result;

        if(!minLogLevel)
        {
            result.addMissing(tagLogger + std::string(LOG_INI_KEY_MIN_LOG_LEVEL));
        }
        return result;
    }

#ifdef USE_SOURCE_INFO
    /**
    * @brief Validates source UUID configuration (if enabled)
    * @return ValidateResult Contains:
    * - success: true if UUID is valid
    * - missingParams: Contains "uuid" if missing when required
    * - invalidParams: Contains "uuid" if format is invalid
    * @details Checks:
    * - UUID is present (if source info is enabled)
    * - UUID follows proper format (if specified)
    * @note Only active when USE_SOURCE_INFO is defined
    */
    ValidateResult Config::validateSource() const
    {
        ValidateResult result;

        if(!sourceName || sourceName->empty())
        {
            result.addMissing(tagSource + std::string(LOG_INI_KEY_SOURCE_NAME));
        }

        if(!sourceUuid)
        {
            result.addMissing(tagSource + std::string(LOG_INI_KEY_SOURCE_UUID));
        }

        if(sourceUuid && !uuids::uuid::is_valid_uuid( * sourceUuid))
        {
            sourceUuid->empty()
            ? result.addMissing(tagSource + std::string(LOG_INI_KEY_SOURCE_UUID))
                    : result.addInvalid(tagSource + std::string(LOG_INI_KEY_SOURCE_UUID), "UUID is not correct: " + * sourceUuid);
        }
#endif
        return result;
    };
};

