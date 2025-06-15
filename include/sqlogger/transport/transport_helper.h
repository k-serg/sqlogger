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

#ifndef TRANSPORT_HELPER_H
#define TRANSPORT_HELPER_H

#include <string>
#include <stdexcept>
#include "sqlogger/internal/log_strings.h"
#include "sqlogger/log_entry.h"
#include "sqlogger/log_helper.h"

/**
 * @def TRANSPORT_TYPE_STR_REST
 * @brief String constant representing REST transport type
 */
#define TRANSPORT_TYPE_STR_REST "REST"

/**
 * @def TRANSPORT_TYPE_STR_GRPC
 * @brief String constant representing gRPC transport type
 */
#define TRANSPORT_TYPE_STR_GRPC "GRPC"

/**
 * @def TRANSPORT_TYPE_STR_WEBSOCKETS
 * @brief String constant representing WebSockets transport type
 */
#define TRANSPORT_TYPE_STR_WEBSOCKETS "WEBSOCKETS"

/**
 * @enum TransportType
 * @brief Enumeration of supported transport protocol types
 */
enum class TransportType
{
    REST,        ///< REST/HTTP transport
    GRPC,        ///< gRPC transport
    WEBSOCKETS   ///< WebSockets transport
};

/**
 * @namespace TransportHelper
 * @brief Provides utilities for working with TransportType conversions
 */
namespace TransportHelper
{
    /**
     * @brief Converts TransportType enum to its string representation
     * @param type The transport type to convert
     * @return std::string String representation of the transport type
     * @throws std::invalid_argument If unknown transport type is provided
     */
    std::string typeToString(const TransportType& type);

    /**
     * @brief Converts string to TransportType enum value
     * @param typeStr String representation of transport type
     * @param ignoreCase When true (default), performs case-insensitive comparison
     * @return TransportType Corresponding transport type enum value
     * @throws std::invalid_argument If string doesn't match any known transport type
     */
    TransportType stringToType(const std::string& typeStr, const bool ignoreCase = true);
};

#endif // !TRANSPORT_HELPER_H
