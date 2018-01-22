

#ifndef __UART_H__
#define __UART_H__

#include "mico.h"
#include "../../include/MiCODrivers/MiCODriverUart.h"

OSStatus uart_init(mico_uart_t port, uint32_t baud);
void uart_recv_thread(mico_thread_arg_t arg);
OSStatus uart_send(mico_uart_t port, const uint8_t *data, uint16_t len);

#endif
