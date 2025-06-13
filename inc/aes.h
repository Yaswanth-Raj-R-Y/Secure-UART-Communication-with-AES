#ifndef AES_H
#define AES_H

#include "stm32f401xc.h"
#include <stdio.h>

//#define KEY "0123456789012345"

/**
 * Functions parameter
 */
uint8_t aes_init(uint8_t* k);
uint8_t* aes_encryption(uint8_t* plain_text);
uint8_t* aes_decryption(uint8_t* encrypted_text);
uint32_t pkcs7_padding(uint8_t* dt, uint32_t dt_len);

#endif