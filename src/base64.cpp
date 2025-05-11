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

#include "base64.h"

/**
* @brief Encodes the given data into a Base64 string.
* @param data The data to be encoded.
* @return The Base64 encoded string.
*/
std::string Base64::base64Encode(const std::vector<unsigned char> & data)
{
    std::string encoded;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    for(const auto & byte : data)
    {
        char_array_3[i++] = byte;
        if(i == 3)
        {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for(i = 0; i < 4; i++)
            {
                encoded += base64_chars[char_array_4[i]];
            }
            i = 0;
        }
    }

    if(i > 0)
    {
        for(j = i; j < 3; j++)
        {
            char_array_3[j] = '\0';
        }

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for(j = 0; j < i + 1; j++)
        {
            encoded += base64_chars[char_array_4[j]];
        }

        while(i++ < 3)
        {
            encoded += '=';
        }
    }

    return encoded;
}

/**
* @brief Decodes the given Base64 string into a vector of unsigned characters.
* @param encoded The Base64 encoded string.
* @return The decoded data as a vector of unsigned characters.
*/
std::vector<unsigned char> Base64::base64Decode(const std::string& encoded)
{
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::vector<unsigned char> decoded;

    auto is_base64 = [](unsigned char c)
    {
        return (isalnum(c) || (c == '+') || (c == '/'));
    };

    int in_len = encoded.size();
    while(in_len-- && encoded[in_] != '=' && is_base64(encoded[in_]))
    {
        char_array_4[i++] = encoded[in_];
        in_++;
        if(i == 4)
        {
            for(i = 0; i < 4; i++)
            {
                char_array_4[i] = static_cast<unsigned char>(base64_chars.find(char_array_4[i]));
            }

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for(i = 0; i < 3; i++)
            {
                decoded.push_back(char_array_3[i]);
            }
            i = 0;
        }
    }

    if(i > 0)
    {
        for(j = i; j < 4; j++)
        {
            char_array_4[j] = 0;
        }

        for(j = 0; j < 4; j++)
        {
            char_array_4[j] = static_cast<unsigned char>(base64_chars.find(char_array_4[j]));
        }

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for(j = 0; j < i - 1; j++)
        {
            decoded.push_back(char_array_3[j]);
        }
    }

    return decoded;
}
