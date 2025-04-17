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
std::string StringHelper::join(const std::vector<std::string>& parts, const std::string& delimiter)
{
    std::string result;
    for (size_t i = 0; i < parts.size(); ++i)
    {
        if (i != 0)
        {
            result += delimiter;
        }
        result += parts[i];
    }
    return result;
}

namespace LogConfig
{
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
        if (!passKey.empty())
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
        if (passKey.has_value())
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
            if(loggerSection.count(LOG_INI_KEY_SYNC_MODE))
            {
                config.syncMode = (loggerSection.at(LOG_INI_KEY_SYNC_MODE) == "true");
            }
            if(loggerSection.count(LOG_INI_KEY_NUM_THREADS))
            {
                config.numThreads = std::stoul(loggerSection.at(LOG_INI_KEY_NUM_THREADS));
            }
            if(loggerSection.count(LOG_INI_KEY_ONLY_FILE_NAMES))
            {
                config.onlyFileNames = (loggerSection.at(LOG_INI_KEY_ONLY_FILE_NAMES) == "true");
            }
            if(loggerSection.count(LOG_INI_KEY_MIN_LOG_LEVEL))
            {
                config.minLogLevel = LogHelper::stringToLevel(loggerSection.at(LOG_INI_KEY_MIN_LOG_LEVEL));
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
                config.databasePort = std::stoi(databaseSection.at(LOG_INI_KEY_DATABASE_PORT));
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
                config.databaseType = DataBaseHelper::stringToDatabaseType(databaseSection.at(LOG_INI_KEY_DATABASE_TYPE));
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
        if (!config.databaseType.has_value())
        {
            throw std::runtime_error("Database type is not specified in config");
        }

        switch (config.databaseType.value())
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
            if (config.databaseHost.has_value())
                parts.emplace_back(std::string(CON_STR_HOST) + "=" + config.databaseHost.value());
            if (config.databaseUser.has_value())
                parts.emplace_back(std::string(CON_STR_USER) + "=" + config.databaseUser.value());
            if (config.databasePass.has_value())
                parts.emplace_back(std::string(CON_STR_PASS) + "=" + config.databasePass.value());
            if (config.databaseName.has_value())
                parts.emplace_back(std::string(CON_STR_DB) + "=" + config.databaseName.value());
            if (config.databasePort.has_value())
                parts.emplace_back(std::string(CON_STR_PORT) + "=" + std::to_string(config.databasePort.value()));

            return StringHelper::join(parts, ";");
        }
        break;

        case DataBaseType::PostgreSQL:
        {
            std::vector<std::string> parts;
            if (config.databaseHost.has_value())
                parts.emplace_back("host=" + config.databaseHost.value());
            if (config.databaseUser.has_value())
                parts.emplace_back("user=" + config.databaseUser.value());
            if (config.databasePass.has_value())
                parts.emplace_back("password=" + config.databasePass.value());
            if (config.databaseName.has_value())
                parts.emplace_back("dbname=" + config.databaseName.value());
            if (config.databasePort.has_value())
                parts.emplace_back("port=" + std::to_string(config.databasePort.value()));

            return StringHelper::join(parts, " ");
        }
        break;

        case DataBaseType::MongoDB:
        {
            std::vector<std::string> parts;
            if (config.databaseHost.has_value())
                parts.emplace_back("host=" + config.databaseHost.value());
            if (config.databaseUser.has_value())
                parts.emplace_back("user=" + config.databaseUser.value());
            if (config.databasePass.has_value())
                parts.emplace_back("password=" + config.databasePass.value());
            if (config.databaseName.has_value())
                parts.emplace_back("database=" + config.databaseName.value());
            if (config.databasePort.has_value())
                parts.emplace_back("port=" + std::to_string(config.databasePort.value()));

            return StringHelper::join(parts, ";");
        }
        break;

        default:
            throw std::runtime_error(ERR_MSG_UNSUPPORTED_DB);
        }
    };
};

