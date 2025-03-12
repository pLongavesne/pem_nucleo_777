/*
 * uart_boot.h
 *
 *  Created on: Mar 11, 2025
 *      Author: pLongavesne
 */
#include "stdint.h"
#ifndef INC_UART_BOOT_H_
#define INC_UART_BOOT_H_

#define RX_BUFFER_SIZE					256
#define TX_BUFFER_SIZE					256

#define UPD_TX_IDLE						0x00
#define UPD_TX_BUSY						0x01


#define BOOT_ACK						0x79
#define BOOT_NACK						0x1F

#define FSM_RX_IDLE						0x00
#define FSM_RX_WAIT_ACK_START			0x01
#define FSM_RX_WAIT_ACK_STOP			0x02
#define FSM_RX_CONFIGURE_RECEIVE		0x03
#define FSM_RX_RECEIVE					0x04
#define FSM_RX_RECEIVE_N				0x05
#define FSM_RX_ACK_ERR					0xEF

#define COMMAND_FIRST_ACK				0x55
#define COMMAND_GET_VERSION				0x56
#define COMMAND_GET_ID					0x57


uint8_t upd_uart_151_mode = 0;


typedef struct UPD_TX_s
{
	uint8_t state;                         // State (IDLE, BUSY, ERROR)
	uint8_t txCount;                         // count down the number of bytes to sent
	uint8_t* ptData;
}UPD_TX_t;
UPD_TX_t txCtx;

typedef struct UPD_RX_s
{
	uint8_t state;                         // fsm state
	uint8_t rxCount;                       // count down the number of bytes to receive
	uint8_t* ptBuff;
	uint8_t command;
}UPD_RX_t;
UPD_RX_t rxCtx;



uint8_t upd_151_program();
void upd_151_uart_irq();

#endif /* INC_UART_BOOT_H_ */
