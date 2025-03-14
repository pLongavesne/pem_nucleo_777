/*
 * uart_boot.h
 *
 *  Created on: Mar 11, 2025
 *      Author: pLongavesne
 */
#include "stdint.h"
#ifndef INC_UART_BOOT_H_
#define INC_UART_BOOT_H_

#define RX_BUFFER_SIZE					512
#define TX_BUFFER_SIZE					512

#define UPD_TX_IDLE						0x00
#define UPD_TX_BUSY						0x01


#define BOOT_ACK						0x79
#define BOOT_NACK						0x1F

#define FSM_RX_IDLE						0x00
#define FSM_RX_WAIT_ACK					0x01
#define FSM_RX_CONFIGURE_RECEIVE		0x03
#define FSM_RX_RECEIVE					0x04
#define FSM_RX_RECEIVE_N				0x05
#define FSM_RX_ACK_ERR					0xEF


#define COMMAND_GET_VERSION				0x56
#define COMMAND_GET_ID					0x57
#define COMMAND_WAIT_ACK				0x58
#define COMMAND_READ					0x59
#define COMMAND_GET						0x60

#define COMMAND_TEST					0x54


#define TIMEOUT_DEFAULT					5000
#define TIMEOUT_READ					10000

#define COM_ERASE_GLOBAL 				0x20
#define COM_ERASE_BANK_1 				0x21
#define COM_ERASE_BANK_2 				0x22



uint8_t upd_uart_151_mode = 0;


typedef struct UPD_TX_s
{
	uint8_t state;                        	 // State (IDLE, BUSY)
	uint32_t txCount;                         // count down the number of bytes to sent
	uint8_t* ptData;
}UPD_TX_t;
UPD_TX_t txCtx;


typedef struct CMD_s
{
	uint8_t *seq;
	uint32_t N;
	uint8_t comCode;
}CMD_t;

typedef struct UPD_RX_s
{
	uint8_t state;                         	// fsm state
	uint8_t stateInd;                       // fsm current state in the sequence for a specific commande
	uint32_t rxCount;                       // count down the number of bytes to receive
	uint8_t* ptBuff;
	CMD_t *command;
}UPD_RX_t;
UPD_RX_t rxCtx;

// sequences of step for the FSM  and communication with the bootloader
const uint8_t COM_WAIT_ACK_SEQ[] = {
		FSM_RX_WAIT_ACK,
		FSM_RX_IDLE};

const uint8_t COM_GET_VERSION_SEQ[] = {
		FSM_RX_WAIT_ACK,
		FSM_RX_RECEIVE,
		FSM_RX_WAIT_ACK,
		FSM_RX_IDLE};

const uint8_t COM_GET_ID_SEQ[] = {
		FSM_RX_WAIT_ACK,
		FSM_RX_RECEIVE_N,
		FSM_RX_RECEIVE,
		FSM_RX_WAIT_ACK,
		FSM_RX_IDLE};

const uint8_t COM_TEST_SEQ[] = {
		FSM_RX_WAIT_ACK,
		FSM_RX_RECEIVE_N,
		FSM_RX_RECEIVE,
		FSM_RX_WAIT_ACK,
		FSM_RX_WAIT_ACK,
		FSM_RX_RECEIVE_N,
		FSM_RX_RECEIVE,
		FSM_RX_WAIT_ACK,
		FSM_RX_IDLE};

const uint8_t COM_READ_SEQ[] = {
		FSM_RX_RECEIVE,
		FSM_RX_IDLE};

const uint8_t COM_GET_SEQ[] = {
		FSM_RX_WAIT_ACK,
		FSM_RX_RECEIVE_N,
		FSM_RX_RECEIVE,
		FSM_RX_WAIT_ACK,
		FSM_RX_IDLE};



uint8_t upd_151_program();
void upd_151_uart_irq();

#endif /* INC_UART_BOOT_H_ */
