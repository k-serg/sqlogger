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

#ifndef BASE64_H
#define BASE64_H

#include <string_view>
#include <vector>
#include <ctype.h>

constexpr std::string_view base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

/**
 * @namespace Base64
 * @brief Provides functionality for Base64 encoding and decoding.
 */
namespace Base64
{
    /**
    * @brief Encodes the given data into a Base64 string.
    * @param data The data to be encoded.
    * @return The Base64 encoded string.
    */
    std::string base64Encode(const std::vector<unsigned char> & data);

    /**
    * @brief Decodes the given Base64 string into a vector of unsigned characters.
    * @param encoded The Base64 encoded string.
    * @return The decoded data as a vector of unsigned characters.
    */
    std::vector<unsigned char> base64Decode(const std::string& encoded);
};

#endif // !BASE64_H
