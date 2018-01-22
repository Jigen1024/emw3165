/*
 * uart.c
 *
 *  Created on: 2018Äê1ÔÂ22ÈÕ
 */

#include "mico.h"
#include "./uart.h"

#define USER_UART_BUFF_LEN          256
#define UART_ONE_PACKAGE_LENGTH     32
#define UART_RECV_TIMEOUT           500

#define uart_log(M, ...) custom_log("UART", M, ##__VA_ARGS__)

volatile ring_buffer_t  rx_buffer;

OSStatus uart_init(mico_uart_t port, uint32_t baud) {
    OSStatus err = kNoErr;
    uint8_t*        rx_data;
    mico_uart_config_t user_uart_config =
    {
        .baud_rate    = baud,
        .data_width   = DATA_WIDTH_8BIT,
        .parity       = NO_PARITY,
        .stop_bits    = STOP_BITS_1,
        .flow_control = FLOW_CONTROL_DISABLED,
        .flags        = UART_WAKEUP_DISABLE,
    };

    rx_data = malloc(USER_UART_BUFF_LEN);
    if(rx_data == NULL) {
        return kNoMemoryErr;
    }
    memset(rx_data, '\0', USER_UART_BUFF_LEN);
    ring_buffer_init( (ring_buffer_t *)&rx_buffer, (uint8_t *)rx_data, USER_UART_BUFF_LEN );
    err = MicoUartInitialize( port, &user_uart_config, (ring_buffer_t *)&rx_buffer );

    return err;
}

OSStatus uart_send(mico_uart_t port, const uint8_t *data, uint16_t len) {
    return MicoUartSend(port, data, len);
}

size_t _uart_get_one_packet(uint32_t port, uint8_t* inBuf, int inBufLen) {
    int datalen;

    while(1) {
        if( MicoUartRecv( port, inBuf, inBufLen, UART_RECV_TIMEOUT) == kNoErr){
            return inBufLen;
        } else {
            datalen = MicoUartGetLengthInBuffer( port );
            if ( datalen ) {
                MicoUartRecv(port, inBuf, datalen, UART_RECV_TIMEOUT);
                return datalen;
            }
        }
    }
}

void uart_recv_thread(mico_thread_arg_t arg) {
    int recvlen;
    uint8_t *inDataBuffer;

    inDataBuffer = malloc(UART_ONE_PACKAGE_LENGTH);
    require(inDataBuffer, exit);

    while(1) {
        recvlen = _uart_get_one_packet(arg, inDataBuffer, UART_ONE_PACKAGE_LENGTH);
        if (recvlen <= 0) {
            continue;
        }
        uart_log("uart recv: ");
        for ( uint8_t i = 0; i < recvlen; i++ ) {
            uart_log("0x%x, ", inDataBuffer[i]);
        }
        uart_log("----------------");
    }
exit:
    if(inDataBuffer) {
        free(inDataBuffer);
    }
    mico_rtos_delete_thread(NULL);
}
