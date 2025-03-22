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

#ifndef LOG_CRYPTO_H
#define LOG_CRYPTO_H

#include <string>
#include <vector>
#include <stdexcept>

#ifdef USE_AES
    #include <openssl/aes.h>
    #include <openssl/evp.h>
    #include <openssl/rand.h>
#endif

#include "base64.h"
#include "log_strings.h"

namespace LogCrypto
{
    /**
     * @brief Encrypts the given data using the specified key.
     * @param data The data to be encrypted.
     * @param key The encryption key.
     * @return The encrypted data as a string.
     */
    std::string encrypt(const std::string& data, const std::string& key);

    /**
     * @brief Decrypts the given data using the specified key.
     * @param data The data to be decrypted.
     * @param key The decryption key.
     * @return The decrypted data as a string.
     */
    std::string decrypt(const std::string& data, const std::string& key);

    /**
     * @brief Encrypts or decrypts the given data using XOR operation with the specified key.
     * @param data The data to be encrypted or decrypted.
     * @param key The key used for XOR operation.
     * @return The result of the XOR operation as a string.
     */
    std::string xorEncryptDecrypt(const std::string& data, const std::string& key);

#ifdef USE_AES
    /**
    * @brief Encrypts the given plaintext using AES-256-CBC encryption.
    * @param plaintext The plaintext to be encrypted.
    * @param key The encryption key.
    * @return The encrypted ciphertext as a vector of unsigned characters.
    */
    std::vector<unsigned char> aesEncrypt(const std::string& plaintext, const std::string& key);

    /**
    * @brief Decrypts the given ciphertext using AES-256-CBC decryption.
    * @param ciphertext The ciphertext to be decrypted.
    * @param key The decryption key.
    * @return The decrypted plaintext as a string.
    */
    std::string aesDecrypt(const std::vector<unsigned char> & ciphertext, const std::string& key);
#endif
}

#endif // !LOG_CRYPTO_H

