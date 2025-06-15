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

#ifndef TRANSPORT_FACTORY_H
#define TRANSPORT_FACTORY_H

#include <memory>
#include "transport_helper.h"
#include "transport_interface.h"

#ifdef SQLG_USE_REST
    #include "sqlogger/transport/backends/rest_transport.h"
#endif

class LogManager; // Forward declaration

/**
 * @class TransportFactory
 * @brief Factory class for creating transport layer implementations
 * @note This is a static factory class - cannot be instantiated or copied
 */
class TransportFactory
{
    public:
        /// Deleted default constructor - pure static class
        TransportFactory() = delete;

        /// Deleted copy constructor - non-copyable
        TransportFactory(const TransportFactory&) = delete;

        /// Deleted assignment operator - non-copyable
        void operator=(const TransportFactory&) = delete;

        // private:

        /**
         * @brief Creates a transport instance of specified type with given configuration
         * @param type The transport protocol type to instantiate
         * @param config Configuration parameters for the transport
         * @return std::unique_ptr<ITransport> Ownership of the created transport instance
         * @throws std::runtime_error If requested transport type is not enabled or implemented
         * @throws std::invalid_argument If unknown transport type is requested
         * @note Actual available transports depend on compile-time definitions:
         * - REST transport requires SQLG_USE_REST definition
         */
        static std::unique_ptr<ITransport> create(const TransportType& type, const LogConfig::Config& config);

        /// Grant exclusive creation rights to LogManager
        friend class LogManager;
};

#endif // !TRANSPORT_FACTORY_H
