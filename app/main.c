#include "main.h"

// Function prototype
void uart1_aes_encrypt(uint8_t* dt, uint32_t dt_len);
void uart1_aes_decrypt(uint8_t* d_dt, uint8_t* s_dt, uint32_t* d_len, uint8_t s_len);

uint8_t uart1_rx(void);
void uart1_tx(uint8_t* tx_dt, uint8_t len);

// Buffers for TX and Rx
uint8_t buffer1[256];
uint32_t buffer1_len;

uint8_t buffer2[256];
uint32_t buffer2_len;

// Message to be encrypted and Tx via UART
char* message = "Confidential: Please keep this information safe.";

int main()
{
    // Initialize system clock, UART 1, UART 2 and AES
    SystemInit();
    uart1_init(115200);
    uart2_init(115200);
    aes_init("0123456789012345");
    
    printf("******** UART with AES encryption ********\n\n");

    printf("Plain text:\n%s\n\n", message);

    // Find length of message
    uint32_t data_len = 0;
    while(message[data_len] != '\0') data_len++;

    // initialize buffer length as zero
    buffer1_len = 0;
    buffer2_len = 0;

    // Encrypt message and Tx over UART 1
    uart1_aes_encrypt((uint8_t*)message, data_len);

    // Print encrypted data
    printf("Tx data (encrypted):\n");
    for (int i=0; i<buffer1_len; i++)
    {
        printf("%02X", buffer1[i]);
    }
    printf("\n\n");

    // Decrypt the received message
    uart1_aes_decrypt(buffer2, buffer1, &buffer2_len, buffer1_len);

    // Print decrypted message
    printf("Rx data (decrypted):\n");
    for (int i=0; i<buffer2_len; i++)
    {
        printf("%c", buffer2[i]);
    }
    printf("\n\n");

    while (1)
    {
        // Do nothing
    }
}

/**
 * @brief This function performs encryption of input data and padding of data if required
 * 
 * @param dt     Input data
 * @param dt_len Length of an input data
 * @return       None
 */
void uart1_aes_encrypt(uint8_t* dt, uint32_t dt_len)
{
    uint8_t tx_dt[16];
    uint8_t tx_index = 0;
    
    // Encrypt 16-bytes of data and transmit 
    for (int i=0; i<dt_len; i++)
    {
        tx_dt[tx_index++] = dt[i];

        if (tx_index == 16)
        {
            aes_encryption(tx_dt);
            uart1_tx(tx_dt, tx_index);
            tx_index = 0;
        }
    }

    // If the data is not 16-byte long use padding and transmit
    if (tx_index != 16)
    {
        tx_index = pkcs7_padding(tx_dt, tx_index);
        aes_encryption(tx_dt);
        uart1_tx(tx_dt, tx_index);
    }
}

/**
 * @brief This function performs decryption of input encrypted data and remove padding if
 *        required
 * 
 * @param d_dt  Destination buffer/addr
 * @param s_dt  Source buffer/data
 * @param d_len Length of destination buffer
 * @param s_len Length of source data
 * @return      None
 */
void uart1_aes_decrypt(uint8_t* d_dt, uint8_t* s_dt, uint32_t* d_len, uint8_t s_len)
{
    for (int i=0; i<s_len; i+=16)
    {
        uint8_t rx[16];
        for (int j=0; j<16; j++)
        {
            rx[j] = s_dt[i+j];
        }
        aes_decryption(rx);
        for (int j=0; j<16; j++)
        {
            d_dt[i+j] = rx[j];
        }
    }

    uint8_t pad_len = d_dt[s_len-1];
    if (pad_len <= 16)
    {
        *d_len = s_len - pad_len;
    }
    else
    {
        *d_len = s_len;
    }
}

/**
 * @brief This function transmit data via UART
 * 
 * @param tx_dt Data to be transmit
 * @param len   Length of the data to be transmit
 * @return      None
 */
void uart1_tx(uint8_t* tx_dt, uint8_t len)
{
    for (int i=0; i<len; i++)
    {
        while (!(USART1->SR & USART_SR_TXE));
        USART1->DR = tx_dt[i] & 0xFF;
    }
}

/**
 * @brief This is an UART IRQ handler which perfoms task to receive incoming data via
 *        uart
 */
void USART1_IRQHandler(void)
{
    if (USART1->SR & USART_SR_RXNE)
        buffer1[buffer1_len++] = USART1->DR;
}