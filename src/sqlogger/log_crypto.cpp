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

#include "sqlogger/log_crypto.h"

namespace LogCrypto
{
#ifdef SQLG_USE_AES
    /**
    * @brief Encrypts the given plaintext using AES-256-CBC encryption.
    * @param plaintext The plaintext to be encrypted.
    * @param key The encryption key.
    * @return The encrypted ciphertext as a vector of unsigned characters.
    */
    std::vector<unsigned char> aesEncrypt(const std::string& plaintext, const std::string& key)
    {
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if(!ctx) throw std::runtime_error(ERR_MSG_CRYPTO_EVPCON_FAILED);

        std::vector<unsigned char> ciphertext(plaintext.size() + AES_BLOCK_SIZE);
        int len = 0;
        int ciphertext_len = 0;

        if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL,
                                   reinterpret_cast<const unsigned char*>(key.c_str()), NULL))
        {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error(ERR_MSG_CRYPTO_ENC_INIT_FAILED);
        }

        if(1 != EVP_EncryptUpdate(ctx, ciphertext.data(), & len,
                                  reinterpret_cast<const unsigned char*>(plaintext.c_str()), plaintext.size()))
        {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error(ERR_MSG_CRYPTO_ENC_UPDATE_FAILED);
        }
        ciphertext_len = len;

        if(1 != EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, & len))
        {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error(ERR_MSG_CRYPTO_ENC_FINAL_FAILED);
        }
        ciphertext_len += len;

        ciphertext.resize(ciphertext_len);
        EVP_CIPHER_CTX_free(ctx);
        return ciphertext;
    }

    /**
    * @brief Decrypts the given ciphertext using AES-256-CBC decryption.
    * @param ciphertext The ciphertext to be decrypted.
    * @param key The decryption key.
    * @return The decrypted plaintext as a string.
    */
    std::string aesDecrypt(const std::vector<unsigned char> & ciphertext, const std::string& key)
    {
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if(!ctx) throw std::runtime_error(ERR_MSG_CRYPTO_EVPCON_FAILED);

        std::vector<unsigned char> plaintext(ciphertext.size());
        int len = 0;
        int plaintext_len = 0;

        if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL,
                                   reinterpret_cast<const unsigned char*>(key.c_str()), NULL))
        {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error(ERR_MSG_CRYPTO_DEC_INIT_FAILED);
        }

        if(1 != EVP_DecryptUpdate(ctx, plaintext.data(), & len, ciphertext.data(), ciphertext.size()))
        {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error(ERR_MSG_CRYPTO_DEC_UPDATE_FAILED);
        }
        plaintext_len = len;

        if(1 != EVP_DecryptFinal_ex(ctx, plaintext.data() + len, & len))
        {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error(ERR_MSG_CRYPTO_DEC_FINAL_FAILED);
        }
        plaintext_len += len;

        plaintext.resize(plaintext_len);
        EVP_CIPHER_CTX_free(ctx);
        return std::string(plaintext.begin(), plaintext.end());
    }
#endif

    /**
    * @brief Encrypts the given data using the specified key.
    * @param data The data to be encrypted.
    * @param key The encryption key.
    * @return The encrypted data as a string.
    */
    std::string encrypt(const std::string& data, const std::string& key)
    {
        if(key.empty())
        {
            throw std::runtime_error(ERR_MSG_PASSKEY_EMPTY);
        }

#ifdef SQLG_USE_AES
        std::vector<unsigned char> ciphertext = aesEncrypt(data, key);
        return Base64::base64Encode(ciphertext);
#else
        std::string xoredText = xorEncryptDecrypt(data, key);
        return Base64::base64Encode(std::vector<unsigned char> (xoredText.begin(), xoredText.end()));
#endif
    }

    /**
    * @brief Decrypts the given data using the specified key.
    * @param data The data to be decrypted.
    * @param key The decryption key.
    * @return The decrypted data as a string.
    */
    std::string decrypt(const std::string& data, const std::string& key)
    {
        if(key.empty())
        {
            throw std::runtime_error(ERR_MSG_PASSKEY_EMPTY);
        }

        std::vector<unsigned char> unbase64 = Base64::base64Decode(data);
#ifdef SQLG_USE_AES
        std::vector<unsigned char> ciphertext(unbase64.begin(), unbase64.end());
        return aesDecrypt(ciphertext, key);
#else
        return xorEncryptDecrypt(std::string(unbase64.begin(), unbase64.end()), key);
#endif
    }

    /**
    * @brief Encrypts or decrypts the given data using XOR operation with the specified key.
    * @param data The data to be encrypted or decrypted.
    * @param key The key used for XOR operation.
    * @return The result of the XOR operation as a string.
    */
    std::string xorEncryptDecrypt(const std::string& data, const std::string& key)
    {
        std::string result = data;
        for(size_t i = 0; i < data.size(); ++i)
        {
            result[i] ^= key[i % key.size()];
        }
        return result;
    }
}
