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

#include "sqlogger/transport/transport_factory.h"

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
std::unique_ptr<ITransport> TransportFactory::create(const TransportType& type, const LogConfig::Config& config)
{
    switch(type)
    {
        case TransportType::REST:
        {
#ifdef SQLG_USE_REST
            return std::make_unique<RestTransport>(config);
#else
            throw std::runtime_error("REST transport is not enabled (SQLG_USE_REST not defined)");
#endif
        }
        break;
        case TransportType::GRPC:
        case TransportType::WEBSOCKETS:
        {
            throw std::runtime_error("Transport type not implemented yet");
        }
        break;
        default:
        {
            throw std::invalid_argument("Unknown transport type");
        }
        break;
    }
}
