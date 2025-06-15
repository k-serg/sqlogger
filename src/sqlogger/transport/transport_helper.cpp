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

#include "sqlogger/transport/transport_helper.h"

/**
 * @brief Converts TransportType enum to its string representation
 * @param type The transport type to convert
 * @return std::string String representation of the transport type
 * @throws std::invalid_argument If unknown transport type is provided
 */
std::string TransportHelper::typeToString(const TransportType& type)
{
    switch(type)
    {
        case TransportType::REST:
            return TRANSPORT_TYPE_STR_REST;

        case TransportType::GRPC:
            return TRANSPORT_TYPE_STR_GRPC;

        case TransportType::WEBSOCKETS:
            return TRANSPORT_TYPE_STR_WEBSOCKETS;

        default:
        {
            throw std::invalid_argument("Unknown transport type");
        }
        break;
    }
}

/**
 * @brief Converts string to TransportType enum value
 * @param typeStr String representation of transport type
 * @param ignoreCase When true (default), performs case-insensitive comparison
 * @return TransportType Corresponding transport type enum value
 * @throws std::invalid_argument If string doesn't match any known transport type
 */
TransportType TransportHelper::stringToType(const std::string& typeStr, const bool ignoreCase)
{
    using namespace LogHelper;
    if((ignoreCase ? toUpperCase(typeStr) : typeStr) == TRANSPORT_TYPE_STR_REST) return TransportType::REST;
    if((ignoreCase ? toUpperCase(typeStr) : typeStr) == TRANSPORT_TYPE_STR_GRPC) return TransportType::GRPC;
    if((ignoreCase ? toUpperCase(typeStr) : typeStr) == TRANSPORT_TYPE_STR_WEBSOCKETS) return TransportType::WEBSOCKETS;
    throw std::invalid_argument("Unknown transport type");
}
