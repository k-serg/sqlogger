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

namespace LogConfig
{
    /**
    * @brief Loads configuration from an INI file.
    * @param filename The path to the INI file.
    * @return A Config object containing the loaded settings.
    * @throws std::runtime_error If the file cannot be parsed.
    */
    Config Config::loadFromINI(const std::string& filename)
    {
        auto iniData = INI::parse(filename);
        Config config;

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
                config.databasePass = databaseSection.at(LOG_INI_KEY_DATABASE_PASS);
            }
            if(databaseSection.count(LOG_INI_KEY_DATABASE_TYPE))
            {
                config.databaseType = DataBaseHelper::stringToDatabaseType(databaseSection.at(LOG_INI_KEY_DATABASE_TYPE));
            }
        }
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
            iniData[LOG_INI_SECTION_DATABASE][LOG_INI_KEY_DATABASE_PASS] = config.databasePass.value();
        }
        if(config.databaseType.has_value())
        {
            iniData[LOG_INI_SECTION_DATABASE][LOG_INI_KEY_DATABASE_TYPE] = DataBaseHelper::databaseTypeToString(config.databaseType.value());
        }

        INI::write(filename, iniData);
    };
};

