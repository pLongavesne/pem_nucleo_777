/*
 * uart_boot.c
 *
 *  Created on: Mar 11, 2025
 *      Author: pLongavesne
 */

#include "uart_boot.h"
#include "stdint.h"
#include "stm32f767xx.h"
#include "stm32f7xx_hal.h"



extern UART_HandleTypeDef huart3;

void upd_enter_bootloader_mode()
{
	//	HAL_GPIO_WritePin(ION_EN_GPIO_Port, ION_EN_Pin, GPIO_PIN_RESET);
	//	HAL_Delay(1000);
	//	HAL_GPIO_WritePin(ION_EN_GPIO_Port, ION_EN_Pin, GPIO_PIN_SET);


	//configure uart for communication with the bootloader :
	// 8E1 : 8 bit + Even parity + 1 stop bit
	USART3->CR1 &= ~USART_CR1_UE;		// disable uart
	USART3->CR1 &= ~USART_CR1_PS;		// even parity
	USART3->CR1 |= USART_CR1_PCE;		// activate parity
	USART3->CR1 |= USART_CR1_M0;		// 9 bit mode needed for parity bit
	USART3->CR1 |= USART_CR1_UE;		// enable uart

	upd_uart_151_mode = 1;

	// enter in boot mode
	//	HAL_GPIO_WritePin(ION_RES_GPIO_Port, ION_RES_Pin, GPIO_PIN_RESET);
	//	HAL_Delay(10);
	//	HAL_GPIO_WritePin(ION_BOOT_GPIO_Port, ION_BOOT_Pin, GPIO_PIN_SET);
	//	HAL_Delay(10);
	//	HAL_GPIO_WritePin(ION_RES_GPIO_Port, ION_RES_Pin, GPIO_PIN_SET);
}

void upd_leave_bootloader_mode()
{
	//configure uart for communication with the chamber :
	// 8n1 : 8 bit + no parity + 1 stop bit
	USART3->CR1 &= ~USART_CR1_UE;		// disable uart
	USART3->CR1 &= ~USART_CR1_PS;		// even parity
	USART3->CR1 &= ~USART_CR1_PCE;		// activate parity
	USART3->CR1 &= ~USART_CR1_M0;		// 8 bit mode
	USART3->CR1 |= USART_CR1_UE;		// enable uart

	upd_uart_151_mode = 0;

	// enter in boot mode
	//	HAL_GPIO_WritePin(ION_RES_GPIO_Port, ION_RES_Pin, GPIO_PIN_RESET);
	//	HAL_Delay(10);
	//	HAL_GPIO_WritePin(ION_BOOT_GPIO_Port, ION_BOOT_Pin, GPIO_PIN_SET);
	//	HAL_Delay(10);
	//	HAL_GPIO_WritePin(ION_RES_GPIO_Port, ION_RES_Pin, GPIO_PIN_SET);

}
void upd_151_uart_irq()
{
	if(huart3.Instance->ISR & USART_ISR_TXE)
	{
		if (txCtx.txCount == 0) //nothing to transmit
		{
			txCtx.state = UPD_TX_IDLE;
			huart3.Instance->CR1 &= ~USART_CR1_TXEIE; //disable irq
		}
		else
		{
			huart3.Instance->TDR = *(txCtx.ptData);
			txCtx.txCount--;
			txCtx.ptData++;
		}
	}

	// receive
	if(huart3.Instance->ISR & USART_ISR_RXNE)
	{
		uint8_t b = huart3.Instance->RDR;
		switch (rxCtx.command)
		{
		case COMMAND_FIRST_ACK:
		{
			switch (rxCtx.state)
			{
			case FSM_RX_WAIT_ACK_START:
			{
				// this byte must be a ack
				if (b != BOOT_ACK)
				{
					rxCtx.state = FSM_RX_ACK_ERR;
				}
				else
				{
					huart3.Instance->CR1 &= ~USART_CR1_RXNEIE; //disable irq on RX
					rxCtx.state = FSM_RX_IDLE;
				}
				break;
			}
			}
			break;
		}
		case COMMAND_GET_VERSION:
		{
			switch (rxCtx.state)
			{
			case FSM_RX_WAIT_ACK_START:
			{
				// this byte must be a ack
				if (b != BOOT_ACK)
				{
					rxCtx.state = FSM_RX_ACK_ERR;
				}
				rxCtx.state = FSM_RX_RECEIVE;
				*rxCtx.ptBuff = b;
				rxCtx.ptBuff++;
				rxCtx.rxCount = 3;
				break;
			}
			case FSM_RX_RECEIVE:
			{
				if (rxCtx.rxCount > 1)
				{
					*rxCtx.ptBuff = b;
					rxCtx.ptBuff++;
					rxCtx.rxCount--;
				}
				else if(rxCtx.rxCount == 1)
				{
					rxCtx.state = FSM_RX_WAIT_ACK_STOP;
					*rxCtx.ptBuff = b;
					rxCtx.ptBuff++;
					rxCtx.rxCount--;
				}

				break;
			}
			case FSM_RX_WAIT_ACK_STOP:
			{
				if (b != BOOT_ACK)
				{
					rxCtx.state = FSM_RX_ACK_ERR;
				}
				else
				{
					*rxCtx.ptBuff = b;
					rxCtx.ptBuff++;
					rxCtx.rxCount--;
					huart3.Instance->CR1 &= ~USART_CR1_RXNEIE; //disable irq on RX
					rxCtx.state = FSM_RX_IDLE;
				}
				break;
			}
			case FSM_RX_IDLE:
			{
				huart3.Instance->CR1 &= ~USART_CR1_RXNEIE; //disable irq
				break;
			}
			case FSM_RX_ACK_ERR:
			{
				// error on a ACK wait
				while(1);
				break;
			}

			}
			break;
		}
		default:
		{
			huart3.Instance->CR1 &= ~USART_CR1_RXNEIE; //disable irq
			break;
		}
		}


	}
}

void upd_151_uart_tx(uint8_t *data, int length)
{
	txCtx.ptData = data;
	txCtx.txCount = length;
	txCtx.state = UPD_TX_BUSY;
	USART3->CR1 |=  USART_CR1_TXEIE;         // enable  IRQ
}

void upd_151_uart_rx(uint8_t *data, int length, uint8_t command)
{
	rxCtx.ptBuff = data;
	rxCtx.rxCount = length;
	rxCtx.state = FSM_RX_WAIT_ACK_START;
	rxCtx.command = command;
	USART3->CR1 |=  USART_CR1_RXNEIE;         // enable  IRQ on receive
}

uint8_t upd_151_program()
{
	uint8_t retVal = HAL_OK;

	//USART3->CR1 |=  USART_CR1_RXNEIE;	// enable receive IRQ
	//USART3->CR1 |=  USART_CR1_RXNEIE;	// enable receive IRQ
	upd_enter_bootloader_mode();

	uint8_t txBuffer[TX_BUFFER_SIZE] = {0x00};
	txBuffer[0] = 0x7f;
	uint8_t rxBuffer[RX_BUFFER_SIZE] = {0x00};


	// first ACK OK
	//	upd_151_uart_rx(rxBuffer, 0, COMMAND_FIRST_ACK);
	//	upd_151_uart_tx(txBuffer, 1);
	//	while(rxCtx.state != FSM_RX_IDLE)
	//	{
	//		//upd_151_uart_tx(txBuffer, 6);
	//		HAL_Delay(100);
	//	}
	txBuffer[0] = 0x01;
	txBuffer[1] = 0xfe;
	upd_151_uart_tx(txBuffer, 2);
	upd_151_uart_rx(rxBuffer, 0, COMMAND_GET_VERSION);
	while(rxCtx.state != FSM_RX_IDLE)
	{
		//upd_151_uart_tx(txBuffer, 6);
		HAL_Delay(100);
	}
	//	while(1)
	//	{
	//		upd_151_uart_tx(txBuffer, 1);
	//
	//		HAL_Delay(100);
	//	}


	upd_leave_bootloader_mode();


	while(1);
	return retVal;
}
