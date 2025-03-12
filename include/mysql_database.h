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

#ifndef MYSQL_DATABASE_H
#define MYSQL_DATABASE_H

#include <mysql.h>
#include <stdexcept>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include "database_interface.h"
#include "database_helper.h"
#include "log_config.h"
#include "log_strings.h"

#ifndef DB_ALLOW_CREATE
    #define DB_ALLOW_CREATE 1
#endif // !DB_ALLOW_CREATE

#ifndef DB_ALLOW_DROP
    #define DB_ALLOW_DROP 0
#endif // !DB_ALLOW_DROP


/**
 * @class MySQLDatabase
 * @brief Implementation of the IDatabase interface for MySQL databases.
 */
class MySQLDatabase : public IDatabase
{
    public:
        /**
         * @brief Constructs a MySQLDatabase object.
         * @param connectionString The connection string in the format "host=...;user=...;password=...;db=...".
         * @throws std::runtime_error if the MySQL library cannot be initialized.
         */
        MySQLDatabase(const std::string& connectionString);

        /**
         * @brief Destructor for MySQLDatabase.
         * Closes the database connection.
         */
        ~MySQLDatabase();

        /**
         * @brief Connects to the MySQL database using the provided connection string.
         * @param connectionString The connection string in the format "host=...;user=...;password=...;database=...".
         * @return True if the connection was successful, false otherwise.
         */
        bool connect(const std::string& connectionString) override;

        /**
         * @brief Disconnects from the MySQL database.
         */
        void disconnect() override;

        /**
         * @brief Checks if the database is connected.
         * @return True if the database is connected, false otherwise.
         */
        bool isConnected() const override;

        /**
         * @brief Executes an SQL query.
         * @param query The SQL query to execute.
         * @return True if the query was executed successfully, false otherwise.
         */
        bool execute(const std::string& query) override;

        /**
         * @brief Executes an SQL query and returns the result.
         * @param query The SQL query to execute.
         * @return A vector of maps representing the query result. Each map contains key-value pairs of column names and their corresponding values.
         */
        std::vector<std::map<std::string, std::string>> query(const std::string& query) override;

        /**
         * @brief Prepares and executes an SQL query with parameters.
         * @param query The SQL query to execute.
         * @param params The parameters to bind to the query.
         * @return True if the query was executed successfully, false otherwise.
         */
        bool executeWithParams(const std::string& query, const std::vector<std::string> & params) override;

        /**
         * @brief Executes an SQL query and returns the number of affected rows.
         * @param query The SQL query to execute.
         * @return The number of affected rows, or -1 if an error occurred.
         */
        int executeWithRowCount(const std::string& query) override;

        /**
         * @brief Begins a transaction.
         * @return True if the transaction was started successfully, false otherwise.
         */
        bool beginTransaction() override;

        /**
         * @brief Commits the current transaction.
         * @return True if the transaction was committed successfully, false otherwise.
         */
        bool commitTransaction() override;

        /**
         * @brief Rolls back the current transaction.
         * @return True if the transaction was rolled back successfully, false otherwise.
         */
        bool rollbackTransaction() override;

        /**
         * @brief Drops the database if it exists.
         *
         * This method attempts to drop the database specified in the connection string.
         * It first disconnects from any existing database connection, initializes a new
         * MySQL connection, and then executes a `DROP DATABASE IF EXISTS` query.
         *
         * @param connectionString The connection string containing database parameters.
         *                         The string should be in the format:
         *                         "host=...;user=...;pass=...;database=...;port=...".
         *
         * @return true if the database was successfully dropped, false otherwise.
         *
         * @note This method is only executed if `DB_ALLOW_DROP` is set to 1.
         *       If `DB_ALLOW_DROP` is 0, the method will return false and set
         *       `errorMessage` to indicate that dropping the database is not allowed.
         *
         * @warning Dropping a database is a destructive operation and cannot be undone.
         *          Use this method with caution.
         */
        bool dropDatabaseIfExists(const std::string& connectionString) override;

        /**
         * @brief Gets the last error message.
         * @return The last error message as a string.
         */
        std::string getLastError() const override;

        /**
         * @brief Gets the type of the database.
         * @return The database type (MySQL in this case).
         */
        DataBaseType getDatabaseType() const override
        {
            return dbType;
        };

    private:
        MYSQL* conn; /**< MySQL connection handle. */

        /**
         * @brief Parses a MySQL connection string.
         * @param connectionString The connection string in the format "host=...;user=...;password=...;database=...".
         * @return A map containing the connection parameters.
         */
        std::map<std::string, std::string> parseConnectionString(const std::string& connectionString);

        /**
        * @brief Creates a database if it does not already exist.
        * @param connectionString The connection string containing database parameters.
        * @return True if the database was created or already exists, false otherwise.
        */
        bool createDatabaseIfNotExists(const std::string& connectionString);

        DataBaseType dbType; /**< The type of the database (MySQL). */

        bool allowCreateDB; /**< Allow create database on server (execute CREATE DATABASE). */
};

#endif // MYSQL_DATABASE_H