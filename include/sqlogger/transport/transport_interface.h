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

#ifndef TRANSPORT_INTERFACE_H
#define TRANSPORT_INTERFACE_H

#include <functional>
#include <cstdint>
#include "sqlogger/logger.h"
#include "sqlogger/log_entry.h"
#include "sqlogger/log_config.h"

/**
 * @brief Transport statistics structure containing connection metrics
 */
struct TransportStats
{
    uint64_t bytesSent = 0;       ///< Total bytes sent through the transport
    uint64_t bytesReceived = 0;   ///< Total bytes received through the transport
    uint32_t activeConnections = 0; ///< Current number of active connections
};

/**
 * @brief Interface for transport layer implementations
 *
 * Defines the contract for log transport mechanisms with support for:
 * - Log pushing/pulling
 * - Configuration updates
 * - Statistics monitoring
 * - Error handling
 */
class ITransport
{
    public:
        /// Handler type for log push operations
        using LogPushHandler = std::function<void(const LogEntry&, std::function<void(bool)>)>;

        /// Handler type for log pull operations
        using LogPullHandler = std::function<void(const std::vector<Filter> &, int, int, std::function<void(LogEntryList)>)>;

        /// Handler type for configuration updates
        using ConfigHandler = std::function<void(const LogConfig::Config&, std::function<void(bool)>)>;

        /// Handler type for error notifications
        using ErrorHandler = std::function<void(const std::string&)>;

        /// Handler type for statistics updates
        using StatsHandler = std::function<void(const SQLogger::Stats&)>;

        /**
         * @brief Starts the transport service
         * @param host The host address to bind/listen to
         * @param port The port number to use
         * @return true if started successfully, false otherwise
         */
        virtual bool start(const std::string& host, uint16_t port) = 0;

        /**
         * @brief Stops the transport service
         */
        virtual void stop() = 0;

        /**
         * @brief Checks if the transport is currently running
         * @return true if transport is active, false otherwise
         */
        virtual bool isRunning() const = 0;

        /**
         * @brief Sets the handler for log push operations
         * @param handler Callback function to handle incoming log pushes
         */
        virtual void setLogPushHandler(LogPushHandler handler) = 0;

        /**
         * @brief Sets the handler for log pull operations
         * @param handler Callback function to handle log retrieval requests
         */
        virtual void setLogPullHandler(LogPullHandler handler) = 0;

        /**
         * @brief Sets the handler for configuration updates
         * @param handler Callback function to handle configuration changes
         */
        virtual void setConfigHandler(ConfigHandler handler) = 0;

        /**
         * @brief Sets the handler for error notifications
         * @param handler Callback function to handle transport errors
         */
        virtual void setErrorHandler(ErrorHandler handler) = 0;

        /**
         * @brief Sets the handler for statistics updates
         * @param handler Callback function to receive statistics
         */
        virtual void setStatsHandler(StatsHandler handler) = 0;

        /**
         * @brief Pushes a log entry through the transport
         * @param entry The log entry to send
         * @param callback Callback to receive operation status (true = success)
         */
        virtual void pushLog(const LogEntry& entry, std::function<void(bool)> callback) = 0;

        /**
         * @brief Retrieves logs matching specified filters
         * @param filters Vector of filters to apply
         * @param limit Maximum number of entries to return
         * @param offset Pagination offset
         * @param callback Callback to receive the resulting LogEntryList
         */
        virtual void pullLogs(const std::vector<Filter> & filters, int limit, int offset,
                              std::function<void(LogEntryList)> callback) = 0;

        /**
         * @brief Publishes statistics through the transport
         * @param stats Statistics data to send
         */
        virtual void pushStats(const SQLogger::Stats& stats) = 0;

        /**
         * @brief Gets current transport statistics
         * @return TransportStats structure with current metrics
         */
        virtual TransportStats getStats() const = 0;

        /// Virtual destructor
        virtual ~ITransport() = default;
};

#endif // !TRANSPORT_INTERFACE_H
