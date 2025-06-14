#include "aes.h"

static uint8_t key[16];
static uint32_t key_expansion[44];

static const uint32_t round_constant[11] =  {   0x00000000, 0x01000000, 0x02000000,
                                                0x04000000, 0x08000000, 0x10000000, 0x20000000, 
                                                0x40000000, 0x80000000, 0x1B000000, 0x36000000
                                            };



static const uint8_t s_box[256] =   {   0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
                                        0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
                                        0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
                                        0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
                                        0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
                                        0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
                                        0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
                                        0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
                                        0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
                                        0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
                                        0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
                                        0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
                                        0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
                                        0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
                                        0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
                                        0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
                                    };

static const uint8_t inv_s_box[256] =   {   0x52, 0x09, 0x6A, 0xD5, 0x30, 0x36, 0xA5, 0x38, 0xBF, 0x40, 0xA3, 0x9E, 0x81, 0xF3, 0xD7, 0xFB,
                                            0x7C, 0xE3, 0x39, 0x82, 0x9B, 0x2F, 0xFF, 0x87, 0x34, 0x8E, 0x43, 0x44, 0xC4, 0xDE, 0xE9, 0xCB,
                                            0x54, 0x7B, 0x94, 0x32, 0xA6, 0xC2, 0x23, 0x3D, 0xEE, 0x4C, 0x95, 0x0B, 0x42, 0xFA, 0xC3, 0x4E,
                                            0x08, 0x2E, 0xA1, 0x66, 0x28, 0xD9, 0x24, 0xB2, 0x76, 0x5B, 0xA2, 0x49, 0x6D, 0x8B, 0xD1, 0x25,
                                            0x72, 0xF8, 0xF6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xD4, 0xA4, 0x5C, 0xCC, 0x5D, 0x65, 0xB6, 0x92,
                                            0x6C, 0x70, 0x48, 0x50, 0xFD, 0xED, 0xB9, 0xDA, 0x5E, 0x15, 0x46, 0x57, 0xA7, 0x8D, 0x9D, 0x84,
                                            0x90, 0xD8, 0xAB, 0x00, 0x8C, 0xBC, 0xD3, 0x0A, 0xF7, 0xE4, 0x58, 0x05, 0xB8, 0xB3, 0x45, 0x06,
                                            0xD0, 0x2C, 0x1E, 0x8F, 0xCA, 0x3F, 0x0F, 0x02, 0xC1, 0xAF, 0xBD, 0x03, 0x01, 0x13, 0x8A, 0x6B,
                                            0x3A, 0x91, 0x11, 0x41, 0x4F, 0x67, 0xDC, 0xEA, 0x97, 0xF2, 0xCF, 0xCE, 0xF0, 0xB4, 0xE6, 0x73,
                                            0x96, 0xAC, 0x74, 0x22, 0xE7, 0xAD, 0x35, 0x85, 0xE2, 0xF9, 0x37, 0xE8, 0x1C, 0x75, 0xDF, 0x6E,
                                            0x47, 0xF1, 0x1A, 0x71, 0x1D, 0x29, 0xC5, 0x89, 0x6F, 0xB7, 0x62, 0x0E, 0xAA, 0x18, 0xBE, 0x1B,
                                            0xFC, 0x56, 0x3E, 0x4B, 0xC6, 0xD2, 0x79, 0x20, 0x9A, 0xDB, 0xC0, 0xFE, 0x78, 0xCD, 0x5A, 0xF4,
                                            0x1F, 0xDD, 0xA8, 0x33, 0x88, 0x07, 0xC7, 0x31, 0xB1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xEC, 0x5F,
                                            0x60, 0x51, 0x7F, 0xA9, 0x19, 0xB5, 0x4A, 0x0D, 0x2D, 0xE5, 0x7A, 0x9F, 0x93, 0xC9, 0x9C, 0xEF,
                                            0xA0, 0xE0, 0x3B, 0x4D, 0xAE, 0x2A, 0xF5, 0xB0, 0xC8, 0xEB, 0xBB, 0x3C, 0x83, 0x53, 0x99, 0x61,
                                            0x17, 0x2B, 0x04, 0x7E, 0xBA, 0x77, 0xD6, 0x26, 0xE1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0C, 0x7D
                                        };


static const uint8_t mix_col[16] =  {   0x02, 0x03, 0x01, 0x01,
                                        0x01, 0x02, 0x03, 0x01,
                                        0x01, 0x01, 0x02, 0x03,
                                        0x03, 0x01, 0x01, 0x02
                                    };

static const uint8_t inv_mix_col[16] =  {   0x0e, 0x0b, 0x0d, 0x09,
                                            0x09, 0x0e, 0x0b, 0x0d,
                                            0x0d, 0x09, 0x0e, 0x0b,
                                            0x0b, 0x0d, 0x09, 0x0e
                                        };

/**
 * @brief Replace each byte in the input data with a corresponding byte from a fixed 
 *        substitution table known as the "S-Box."
 * 
 * @param state input data
 * @param size  length of AES key (128-bit)
 * @return      None
 */
static void sub_bytes(uint8_t* state, uint8_t size)
{
    for (int i=0; i<size; i++)
    {
        uint8_t upper_nibble = (state[i] >> 4) & 0xF;
        uint8_t lower_nibble = state[i] & 0xF;

        state[i] = s_box[(upper_nibble*16) + lower_nibble];
    }
}

/**
 * @brief This function compute G transformation for AES key expansion. It performs a left 
 *        rotate, applies s-box substitution and XOR with the round constand.
 * 
 * @param word  32-bit input word to be transformed.
 * @param r     Round number.
 * @return      Transformed 32-bit word.
 */
static uint32_t compute_g(uint32_t word, uint8_t r)
{
    // Perform left rotate
    uint8_t lsb = (word >> 24) & 0xFF;
    word = word << 8;
    word = word | lsb;
    
    // Apply S-box substitution
    uint8_t* base = (uint8_t*)&word;
    sub_bytes(base, 4);

    return (word ^ round_constant[r]);
}

/**
 * @brief Expands a 128-bit AES key into 44 round keys (11 rounds)
 * 
 * @param word Output buffer for expanded keys (44x uint32_t)
 * @param k    Input 128-bit key (16 bytes)
 * @return     None
 */
static void expand_key(uint32_t* word, uint8_t* k)
{
    // Convert 16-byte key to 4 initial 32-bit words
    for (int i=0; i<4; i++)
    {
        // Pack 4-bytes into one word
        word[i] = (k[i * 4] << 24) | (k[i * 4 + 1] << 16) | (k[i * 4 + 2] << 8) | (k[i * 4 + 3]);
    }

    // Generate 40 additional words (10 rounds)
    uint8_t round = 0;
    for (int i=4; i<44; i+=4)
    {
        // Compute g with previous word (4-bytes)
        uint32_t g = compute_g(word[i-1], ++round);

        // Calculate new 4-word group:
        word[i] = word[i - 4] ^ g;
        word[i + 1] = word[i + 1 - 4] ^ word[i];
        word[i + 2] = word[i + 2 - 4] ^ word[i + 1];
        word[i + 3] = word[i + 3 - 4] ^ word[i + 2];
    }
}

/**
 * @brief This function checks the AES key length (128-bits), and expands the key to generate 
 *        the round keys for encryption and decryption.
 * 
 * @param k 16-byte (128-bit) key value for encryption
 * @return  0 if successfully installed or -1 if unsuccessful
 */
uint8_t aes_init(uint8_t* k)
{
    int len = 0;
    while (k[len] != '\0') 
    {
        key[len] = k[len];
        len++;
    }

    if (len != 16)
    {
        printf(" %d Invalid key length! Please enter a 128-bit (16-byte) key for AES-128.\n", len);
        return -1; // return failed
    }

    // Expand the key
    expand_key(key_expansion, key);

    return 0; // return success
}

/*---------------------------------------------------------------------------------------------*/

/**
 * @brief This function performs XOR operation with the column of input matrix and 32-bit word of
 *        round key.
 * 
 * @param state Input matrix (16-bytes)
 * @param round index of round constant in AES 128-bit.
 * @return      None
 */
static void add_round_key(uint8_t* state, uint8_t round)
{
    for (int i = 0; i < 4; i++) 
    {
        uint32_t k = key_expansion[round * 4 + i];

        state[i*4 + 0] ^= (k >> 24) & 0xff;
        state[i*4 + 1] ^= (k >> 16) & 0xff;
        state[i*4 + 2] ^= (k >> 8) & 0xff;
        state[i*4 + 3] ^= (k >> 0) & 0xff;
    }
}

/**
 * @brief This function performs left shift operation in input matrix. no left shift in first row,
 *        one left shift in secound row, two left shift in third row and 3 left shift in fourth row.
 * 
 * @param state Input matrix
 * @return      None
 */
static void shift_rows(uint8_t* state)
{
    uint8_t tmp[16];

    // Row 0 (no shift)
    tmp[0]  = state[0];
    tmp[4]  = state[4];
    tmp[8]  = state[8];
    tmp[12] = state[12];

    // Row 1 (shift left by 1)
    tmp[1]  = state[5];
    tmp[5]  = state[9];
    tmp[9]  = state[13];
    tmp[13] = state[1];

    // Row 2 (shift left by 2)
    tmp[2]  = state[10];
    tmp[6]  = state[14];
    tmp[10] = state[2];
    tmp[14] = state[6];

    // Row 3 (shift left by 3)
    tmp[3]  = state[15];
    tmp[7]  = state[3];
    tmp[11] = state[7];
    tmp[15] = state[11];

    // Copy back
    for (int i = 0; i < 16; i++)
    {
        state[i] = tmp[i];
    } 
}

/**
 * @brief This function performs matrix multiplication using galios field.
 * 
 * @param a Column value of mix_col matrix
 * @param b Row value of input matrix
 * @return  Multiplied value
 */
static uint8_t gfmul(uint8_t a, uint8_t b) {
    uint8_t p = 0;

    for (int i = 0; i < 8; i++)
    {
        if (b & 1) 
        {
            p ^= a;
        }

        uint8_t hi_bit_set = a & 0x80;
        a <<= 1;

        if (hi_bit_set) 
        {
            a ^= 0x1b;    
        }
        b >>= 1;
    }
    return p;
}

/**
 * @brief This function performs simple matrix multiplication of input matrix with predefined
 *        matrix
 * 
 * @param state Input matrix
 * @return      None
 */
static void mix_columns(uint8_t* state)
{
    uint8_t tmp[16];

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            tmp[i * 4 + j] = 0;

            for (int k = 0; k < 4; k++)
            {
                tmp[i * 4 + j] ^= gfmul(mix_col[j * 4 + k], state[i * 4 + k]);
            }
        }
    }

    // Copy back to base
    for (int i = 0; i < 16; ++i)
    {
        state[i] = tmp[i];
    }
}

/**
 * @brief This function encrypt the block of data by performing steps like sub_bytes
 *        shift_rows,mix_columns and add_round_key for 10 rounds.

 * @param plain_text 16-bytes of data that needs to be encrypted.
 * @return           Encrypted data
 */
uint8_t* aes_encryption(uint8_t* plain_text)
{
    // Round 0
    add_round_key(plain_text, 0);

    // Round 1-9
    for (int i=1; i<10; i++)
    {
        sub_bytes(plain_text, 16);
        shift_rows(plain_text);
        mix_columns(plain_text);
        add_round_key(plain_text, i);
    }

    // Round 10
    sub_bytes(plain_text, 16);
    shift_rows(plain_text);
    add_round_key(plain_text, 10);

    // Return encrypted data
    return plain_text;
}

/*---------------------------------------------------------------------------------------------*/

/**
 * @brief Replace each byte in the input data with a corresponding byte from a fixed 
 *        substitution table known as the "S-Box."
 * 
 * @param state input data
 * @param size  length of AES key (128-bit)
 * @return      None
 */
static void inv_sub_bytes(uint8_t* state, uint8_t size)
{
    for (int i=0; i<size; i++)
    {
        uint8_t upper_nibble = (state[i] >> 4) & 0xF;
        uint8_t lower_nibble = state[i] & 0xF;

        state[i] = inv_s_box[(upper_nibble*16) + lower_nibble];
    }
}

/**
 * @brief This function performs right shift operation in input matrix. no right shift in first row,
 *        one right shift in secound row, two right shift in third row and 3 right shift in fourth row.
 * 
 * @param state Input matrix
 * @return      None
 */
static void inv_shift_rows(uint8_t* state)
{
    uint8_t tmp[16];

    // Row 0 (no shift)
    tmp[0]  = state[0];
    tmp[4]  = state[4];
    tmp[8]  = state[8];
    tmp[12] = state[12];

    // Row 1 (shift right by 1)
    tmp[1]  = state[13];
    tmp[5]  = state[1];
    tmp[9]  = state[5];
    tmp[13] = state[9];

    // Row 2 (shift right by 2)
    tmp[2]  = state[10];
    tmp[6]  = state[14];
    tmp[10] = state[2];
    tmp[14] = state[6];

    // Row 3 (shift right by 3)
    tmp[3]  = state[7];
    tmp[7]  = state[11];
    tmp[11] = state[15];
    tmp[15] = state[3];

    // Copy back
    for (int i = 0; i < 16; i++)
    {
        state[i] = tmp[i];
    } 
}

/**
 * @brief This function performs simple matrix multiplication of input matrix with predefined
 *        matrix
 * 
 * @param state Input matrix
 * @return      None
 */
static void inv_mix_columns(uint8_t* state)
{
    uint8_t temp[16];

    for (int col = 0; col < 4; ++col)
    {
        for (int row = 0; row < 4; ++row)
        {
            temp[col * 4 + row] = 0;
            for (int k = 0; k < 4; ++k)
            {
                temp[col * 4 + row] ^= gfmul(inv_mix_col[row * 4 + k], state[col * 4 + k]);
            }
        }
    }
    // Copy back to base
    for (int i = 0; i < 16; ++i)
    {
        state[i] = temp[i];
    }
}

/**
 * @brief This function decrypte the encrypted data by performing 10 rounds of same steps as 
          encrytion in inverse.
 * 
 * @param state 16-byte encrypted data
 * @return      16-byte of decrypted
 */
uint8_t* aes_decryption(uint8_t* encrypted_text)
{   
    // Round 0
    add_round_key(encrypted_text, 10);

    // Round 1-9
    for (int i=9; i>0; i--)
    {
        inv_sub_bytes(encrypted_text, 16);
        inv_shift_rows(encrypted_text);
        add_round_key(encrypted_text, i);
        inv_mix_columns(encrypted_text);
    }

    // Round 10
    inv_shift_rows(encrypted_text);
    inv_sub_bytes(encrypted_text, 16);
    add_round_key(encrypted_text, 0);

    // Return decrypted data
    return encrypted_text;
}

/*---------------------------------------------------------------------------------------------*/

/**
 * @brief This function performs PKCS7 padding. If the data is not 16-bytes length long it adds
 *        padding to the remaing length with the value of padding size.
 * 
 * @param dt        Input data to be padded with.
 * @param dt_len    Total length of dt.
 * @return          Total length of data after padded.
 */
uint32_t pkcs7_padding(uint8_t* dt, uint32_t dt_len)
{
    // Find number of padding required
    uint32_t pad_len = 16 - (dt_len % 16);

    if (pad_len == 0) pad_len = 16; // return if 0

    // Add number padding in remaining bytes
    for (int i=0; i<pad_len; i++)
    {
        dt[dt_len + i] = pad_len;
    }

    // return new length after padding
    return dt_len + pad_len;
}