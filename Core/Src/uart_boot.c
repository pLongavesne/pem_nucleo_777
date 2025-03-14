/*
 * uart_boot.c
 *
 *  Created on: Mar 11, 2025
 *      Author: pLongavesne
 */

#include "uart_boot.h"
#include "stdint.h"
#include "string.h"
#include "stm32f767xx.h"
#include "stm32f7xx_hal.h"



extern UART_HandleTypeDef huart3;

uint8_t upd_151_boot_erase_extended(uint16_t *pageNumbers, uint16_t nPage);
uint8_t upd_151_boot_erase(uint8_t *pageNumbers, uint8_t nPage);
uint8_t upd_151_boot_read_memory(uint32_t addr, uint16_t nByte, uint8_t *readBuffer);
uint8_t upd_wait_rx_idle_timeout(uint32_t timeout);
uint8_t upd_151_boot_write_memory(uint32_t addr, uint16_t nBytes, uint8_t *data);
uint8_t upd_151_boot_special_erase(uint8_t mode);

/*
 * Reset the STM32l151 to enter in bootloader mode
 * Configure the uart to work with the bootloader of the STM32l151 (8E1)
 */
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

/*
 * Restore the original configuration of the UART (8N1)
 * Reset the STM32l151 to boot in user code
 */
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
	// transmit
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
		//		if(b == 0x02)
		//		{
		//			printf("%x",b);
		//		}
		switch (rxCtx.state)
		{
		case FSM_RX_WAIT_ACK:
		{
			// this byte must be a ack
			if (b != BOOT_ACK)
			{
				rxCtx.state = FSM_RX_ACK_ERR;
			}
			else
			{
				//rxCtx.state = FSM_RX_RECEIVE_N;
				rxCtx.stateInd++;
				rxCtx.state = rxCtx.command->seq[rxCtx.stateInd];

				*rxCtx.ptBuff = b;
				rxCtx.ptBuff++;
			}
			break;
		}
		case FSM_RX_RECEIVE_N:
		{
			*rxCtx.ptBuff = b;
			rxCtx.ptBuff++;
			rxCtx.rxCount = b+1;
			rxCtx.stateInd++;
			rxCtx.state = rxCtx.command->seq[rxCtx.stateInd];
			//			rxCtx.state = FSM_RX_RECEIVE;
			break;
		}
		case FSM_RX_RECEIVE:
		{
			*rxCtx.ptBuff = b;
			rxCtx.ptBuff++;
			rxCtx.rxCount--;
			if (rxCtx.rxCount == 0)
			{
				rxCtx.stateInd++;
				rxCtx.state = rxCtx.command->seq[rxCtx.stateInd];
				//				rxCtx.state = FSM_RX_WAIT_ACK_STOP;

			}
			break;
		}
		case FSM_RX_ACK_ERR:
		{
			break;
		}
		case FSM_RX_IDLE:
		{
			break;
		}
		break;
		default:
		{
			// error on a FSM state
			while(1);
			break;
		}

		}



	}
}




void upd_151_uart_tx(uint8_t *data, int length)
{
	while(txCtx.state != UPD_TX_IDLE)
	{
		HAL_Delay(1);
	}


	txCtx.ptData = data;
	txCtx.txCount = length;
	txCtx.state = UPD_TX_BUSY;
	USART3->CR1 |=  USART_CR1_TXEIE;         // enable  IRQ
}

uint8_t upd_151_uart_rx(uint8_t *data, CMD_t *command)
{
	if (rxCtx.state == FSM_RX_IDLE)
	{
		rxCtx.ptBuff = data;
		rxCtx.rxCount = command->N;
		rxCtx.state = command->seq[0];

		rxCtx.stateInd = 0;
		rxCtx.command = command;
		USART3->CR1 |=  USART_CR1_RXNEIE;         // enable  IRQ on receive
		return HAL_OK;
	}
	else
	{
		return HAL_ERROR;
	}
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



	retVal = upd_151_boot_special_erase(COM_ERASE_GLOBAL);

	if (retVal == HAL_ERROR)
	{
		return HAL_ERROR;
	}
	retVal = upd_151_boot_special_erase(COM_ERASE_BANK_1);

	if (retVal == HAL_ERROR)
	{
		return HAL_ERROR;
	}
	retVal = upd_151_boot_special_erase(COM_ERASE_BANK_2);

	if (retVal == HAL_ERROR)
	{
		return HAL_ERROR;
	}

	// first ACK OK
	//	upd_151_uart_rx(rxBuffer, 0, COMMAND_FIRST_ACK);
	//	upd_151_uart_tx(txBuffer, 1);
	//	while(rxCtx.state != FSM_RX_IDLE)
	//	{
	//		HAL_Delay(100);
	//	}

	//*************** commande get version OK ***********************
	//	CMD_t comGetVersion;
	//	comGetVersion.N = 3;
	//	comGetVersion.comCode = COMMAND_GET_VERSION;
	//	comGetVersion.seq = COM_GET_VERSION_SEQ;
	//	txBuffer[0] = 0x01;
	//	txBuffer[1] = 0xfe;
	//	upd_151_uart_tx(txBuffer, 2);
	//	upd_151_uart_rx(rxBuffer, &comGetVersion);
	//	retVal = upd_wait_rx_idle_timeout();
	//	if (retVal == HAL_ERROR)
	//	{
	//		return HAL_ERROR;
	//	}
	//***************************************************************


	//***************** commande get ID OK **************************
	//	memset(rxBuffer, 0x00, RX_BUFFER_SIZE);
	//	CMD_t comGetId;
	//	comGetId.N = 0;
	//	comGetId.comCode = COMMAND_GET_ID;
	//	comGetId.seq = COM_GET_ID_SEQ;
	//
	//	// commande get ID OK
	//	txBuffer[0] = 0x02;
	//	txBuffer[1] = 0xfd;
	//	upd_151_uart_tx(txBuffer, 2);
	//	upd_151_uart_rx(rxBuffer, &comGetId);
	//	retVal = upd_wait_rx_idle_timeout();
	//	if (retVal == HAL_ERROR)
	//	{
	//		return HAL_ERROR;
	//	}
	//***************************************************************

	//************** commande erase memory OK ***********************
	//	uint8_t pageToErase[] = {1,2,3,4,5};
	//	retVal = upd_151_boot_erase(pageToErase, sizeof(pageToErase));
	//	if (retVal == HAL_ERROR)
	//	{
	//		return HAL_ERROR;
	//	}
	//***************************************************************

	//*************** commande read memory OK ***********************
	//	retVal = upd_151_boot_read_memory(0x01020304, 10, rxBuffer);
	//	if (retVal == HAL_ERROR)
	//	{
	//		return HAL_ERROR;
	//	}
	//***************************************************************

	//*************** commande write memory  OK ***********************
	//	memmove(txBuffer, "abcdefghij",10);
	//	retVal = upd_151_boot_write_memory(0x01020304, 10, txBuffer);
	//	if (retVal == HAL_ERROR)
	//	{
	//		return HAL_ERROR;
	//	}
	//***************************************************************


	//******************** Command Get OK ******************************
	//
	//	CMD_t comGet;
	//	comGet.N = 0;
	//	comGet.comCode = COMMAND_GET;
	//	comGet.seq = COM_GET_SEQ;
	//
	//	// commande get
	//	txBuffer[0] = 0x00;
	//	txBuffer[1] = 0xff;
	//	upd_151_uart_tx(txBuffer, 2);
	//	upd_151_uart_rx(rxBuffer, &comGet);
	//	retVal = upd_wait_rx_idle_timeout(TIMEOUT_DEFAULT);
	//	if (retVal == HAL_ERROR)
	//	{
	//		return HAL_ERROR;
	//	}
	//
	//	while(1);

	//***************************************************************

	//**************** Command Extended Erase ***********************
	uint16_t pageToErase[] = {1,2,3,4,5};
	retVal = upd_151_boot_erase_extended(pageToErase, sizeof(pageToErase)/2);
	if (retVal == HAL_ERROR)
	{
		return HAL_ERROR;
	}
	//***************************************************************
	upd_leave_bootloader_mode();


	while(1);
	return retVal;
}
/*
 * Write maximum 256 bytes at addr
 */
uint8_t upd_151_boot_write_memory(uint32_t addr, uint16_t nBytes, uint8_t *data)
{
	uint8_t retVal = HAL_OK;
	uint8_t txBuffer[TX_BUFFER_SIZE+1] = {0x00};
	uint8_t rxBuffer[RX_BUFFER_SIZE] = {0x00};

	// ********************** send command and xor of the command **********************
	txBuffer[0] = 0x31;
	txBuffer[1] = 0xCE;
	upd_151_uart_tx(txBuffer, 2);

	// ********************** wait for ack with timeout **********************
	CMD_t comWaitAck;
	comWaitAck.N = 0;
	comWaitAck.comCode = COMMAND_WAIT_ACK;
	comWaitAck.seq = COM_WAIT_ACK_SEQ;
	upd_151_uart_rx(rxBuffer, &comWaitAck);
	retVal = upd_wait_rx_idle_timeout(TIMEOUT_DEFAULT);
	if (retVal == HAL_ERROR)
	{
		return HAL_ERROR;
	}

	//********************** send the addresse to write **********************
	uint32_t msk = 0xff000000;	// init the mask to parse the address
	uint8_t crc = 0x00;			//init the crc
	for (int i=0; i<4; i++)
	{
		uint32_t b = (addr & msk);
		txBuffer[i] = (b>>((3-i)*8));
		msk = msk >> 8;
		crc ^= txBuffer[i];
	}
	txBuffer[4] = crc;			//adding the crc at the end of the buffer
	upd_151_uart_tx(txBuffer, 5);


	// ********************** wait for ack with timeout **********************
	upd_151_uart_rx(rxBuffer, &comWaitAck);
	retVal = upd_wait_rx_idle_timeout(TIMEOUT_DEFAULT);
	if (retVal == HAL_ERROR)
	{
		return HAL_ERROR;
	}

	// ********************** sending data to write	 **********************
	memset(txBuffer, 0x00, TX_BUFFER_SIZE);
	txBuffer[0] = nBytes-1;			// adding number of byte to write at the beginning of the buffer
	crc = txBuffer[0];				// init crc
	for(int i=0; i<=nBytes; i++)		// adding data to write in the buffer
	{
		txBuffer[i+1] = data[i];
		crc ^= data[i];
	}
	txBuffer[nBytes+1] = crc;		//adding the crc at the end of the buffer
	upd_151_uart_tx(txBuffer, nBytes+1+1);	//sending nBytes + crc + data

	// ********************** wait for ack with timeout **********************
	upd_151_uart_rx(rxBuffer, &comWaitAck);
	retVal = upd_wait_rx_idle_timeout(TIMEOUT_DEFAULT);
	if (retVal == HAL_ERROR)
	{
		return HAL_ERROR;
	}

	return retVal;
}

/*
 * Wait the RX to do idle
 * Also check if there is not NACK error
 */
uint8_t upd_wait_rx_idle_timeout(uint32_t timeout){
	uint32_t t = HAL_GetTick();
	while(rxCtx.state != FSM_RX_IDLE)
	{
		if((HAL_GetTick() - t) > timeout || rxCtx.state == FSM_RX_ACK_ERR)
		{
			return HAL_ERROR;
		}
	}
	return HAL_OK;
}


/*
 * Read maximum 256 bytes from addr
 */
uint8_t upd_151_boot_read_memory(uint32_t addr, uint16_t nByte, uint8_t *readBuffer)
{
	uint8_t retVal = HAL_OK;
	uint8_t txBuffer[TX_BUFFER_SIZE] = {0x00};
	uint8_t rxBuffer[RX_BUFFER_SIZE] = {0x00};

	// ********************** send command and xor of the command **********************
	txBuffer[0] = 0x11;
	txBuffer[1] = 0xEE;
	upd_151_uart_tx(txBuffer, 2);

	// ********************** wait for ack with timeout **********************
	CMD_t comWaitAck;
	comWaitAck.N = 0;
	comWaitAck.comCode = COMMAND_WAIT_ACK;
	comWaitAck.seq = COM_WAIT_ACK_SEQ;
	upd_151_uart_rx(rxBuffer, &comWaitAck);
	retVal = upd_wait_rx_idle_timeout(TIMEOUT_DEFAULT);
	if (retVal == HAL_ERROR)
	{
		return HAL_ERROR;
	}

	//********************** send the addresse to read **********************
	uint32_t msk = 0xff000000;	// init the mask to parse the address
	uint8_t crc = 0x00;			//init the crc
	for (int i=0; i<4; i++)
	{
		uint32_t b = (addr & msk);
		txBuffer[i] = (b>>((3-i)*8));
		msk = msk >> 8;
		crc ^= txBuffer[i];
	}
	txBuffer[4] = crc;			//adding the crc at the end of the buffer
	upd_151_uart_tx(txBuffer, 5);

	// ********************** wait for ack with timeout **********************
	upd_151_uart_rx(rxBuffer, &comWaitAck);
	retVal = upd_wait_rx_idle_timeout(TIMEOUT_DEFAULT);
	if (retVal == HAL_ERROR)
	{
		return HAL_ERROR;
	}

	// ********************** sending the number of bytes to read **********************
	memset(txBuffer, 0x00, TX_BUFFER_SIZE); //clearing the tx buffer
	txBuffer[0] = nByte-1;				//nb of byte to read -1
	txBuffer[1] = (nByte-1)^0xff;		//complement xor
	upd_151_uart_tx(txBuffer, 2);

	// ********************** receiving the bytes to read **********************
	CMD_t comRead;
	comRead.N = nByte;
	comRead.comCode = COMMAND_READ;
	comRead.seq = COM_READ_SEQ;
	upd_151_uart_rx(readBuffer, &comRead);

	// wait for receive complete with timeout
	retVal = upd_wait_rx_idle_timeout(TIMEOUT_DEFAULT);
	if (retVal == HAL_ERROR)
	{
		return HAL_ERROR;
	}


	return retVal;
}

/*
 *	$$ Erase N=nPage pages 0<=N<0xfff0
 *	For N > 0xfff0 special command are made
 *	$$ PageNumbers specify the number of the pages to erase
 *	16bit page numerous MSB first
 *
 *	There is 2048 pages in the STM32L151RET6
 *	maximum nPage = 2048
 */
uint8_t upd_151_boot_erase_extended(uint16_t *pageNumbers, uint16_t nPage)
{
	uint8_t retVal;

	if (nPage > 2048)	// number of page in the STM32L151RET6
	{
		return HAL_ERROR;
	}

	//send command and the xor of the command
	uint16_t txBuffer[TX_BUFFER_SIZE] = {0x00};
	uint16_t rxBuffer[RX_BUFFER_SIZE] = {0x00};
	txBuffer[0] = 0x43BC;
	upd_151_uart_tx(txBuffer, 2);

	// wait for ack with timeout
	CMD_t comWaitAck;
	comWaitAck.N = 0;
	comWaitAck.comCode = COMMAND_WAIT_ACK;
	comWaitAck.seq = COM_WAIT_ACK_SEQ;
	upd_151_uart_rx(rxBuffer, &comWaitAck);
	retVal = upd_wait_rx_idle_timeout(TIMEOUT_DEFAULT);
	if (retVal == HAL_ERROR)
	{
		return HAL_ERROR;
	}

	// creation of the command buffer to send
	txBuffer[0] = nPage;	// adding number of page to erase
	uint8_t crc = txBuffer[0];	// init crc

	for (int i=0; i<nPage; i++)	// adding page number
	{
		txBuffer[i+1] = pageNumbers[i];
		crc ^= pageNumbers[i];
	}
	txBuffer[nPage+1] = crc;	// adding crc

	//sending the buffer
	//number of page + pages numbers + checksum
	upd_151_uart_tx(txBuffer, nPage*2+2+1);

	//wait for ack with timeout
	upd_151_uart_rx(rxBuffer, &comWaitAck);
	retVal = upd_wait_rx_idle_timeout(TIMEOUT_DEFAULT);
	if (retVal == HAL_ERROR)
	{
		return HAL_ERROR;
	}

	return retVal;
}

/*
 *	Erase N=nPage pages
 *	pageNumbers specify the number of the pages to erase
 */
uint8_t upd_151_boot_erase(uint8_t *pageNumbers, uint8_t nPage)
{
	uint8_t retVal = HAL_OK;

	//send command and the xor of the command
	uint8_t txBuffer[TX_BUFFER_SIZE] = {0x00};
	uint8_t rxBuffer[RX_BUFFER_SIZE] = {0x00};
	txBuffer[0] = 0x43;
	txBuffer[1] = 0xBC;
	upd_151_uart_tx(txBuffer, 2);

	// wait for ack with timeout
	CMD_t comWaitAck;
	comWaitAck.N = 0;
	comWaitAck.comCode = COMMAND_WAIT_ACK;
	comWaitAck.seq = COM_WAIT_ACK_SEQ;
	upd_151_uart_rx(rxBuffer, &comWaitAck);
	retVal = upd_wait_rx_idle_timeout(TIMEOUT_DEFAULT);
	if (retVal == HAL_ERROR)
	{
		return HAL_ERROR;
	}

	// creation of the command buffer to send
	txBuffer[0] = nPage - 1;	// adding number of page to erase
	uint8_t crc = txBuffer[0];	// init crc

	for (int i=0; i<nPage; i++)	// adding page number
	{
		txBuffer[i+1] = pageNumbers[i];
		crc ^= pageNumbers[i];
	}
	txBuffer[nPage+1] = crc;	// adding crc

	//sending the buffer
	//number of page + pages numbers + checksum
	upd_151_uart_tx(txBuffer, nPage+2);

	//wait for ack with timeout
	upd_151_uart_rx(rxBuffer, &comWaitAck);
	retVal = upd_wait_rx_idle_timeout(TIMEOUT_DEFAULT);
	if (retVal == HAL_ERROR)
	{
		return HAL_ERROR;
	}
	return retVal;
}

/*
 * Sending the an special erase command to the bootloader
 * INPUT :
 * 		- mode : select the special erase commande
 * 				-> COM_ERASE_GLOBAL : global Erase
 * 				-> COM_ERASE_BANK_1 : bank 1 erase
 * 				-> COM_ERASE_BANK_2 : bank 2 erase
 * OUTPUT :
 *  	- HAL_OK : flash memory erased OK
 *  	- HAL_ERROR : unknow mode parameter
 *  		or timeout or nack from bootloader
 */
uint8_t upd_151_boot_special_erase(uint8_t mode)
{
	uint8_t retVal = HAL_OK;

	uint16_t txBuffer[TX_BUFFER_SIZE] = {0x00};
	uint16_t rxBuffer[RX_BUFFER_SIZE] = {0x00};


	//send command and the xor of the command
	txBuffer[0] = 0xBB44;
	upd_151_uart_tx(txBuffer, 2);

	// wait for ack with timeout
	CMD_t comWaitAck;
	comWaitAck.N = 0;
	comWaitAck.comCode = COMMAND_WAIT_ACK;
	comWaitAck.seq = COM_WAIT_ACK_SEQ;
	upd_151_uart_rx(rxBuffer, &comWaitAck);
	retVal = upd_wait_rx_idle_timeout(TIMEOUT_DEFAULT);
	if (retVal == HAL_ERROR)
	{
		return HAL_ERROR;
	}

	// sending the erase command
	if(mode == COM_ERASE_GLOBAL)
	{
		txBuffer[0] = 0xFFFF;
	}
	else if (mode == COM_ERASE_BANK_1)
	{
		txBuffer[0] = 0xFEFF;
	}
	else if (mode == COM_ERASE_BANK_2)
	{
		txBuffer[0] = 0xFDFF;
	}
	else
	{
		return HAL_ERROR;
	}
	txBuffer[1] = (txBuffer[0] >> 8)^txBuffer[0];
	upd_151_uart_tx(txBuffer, 3);


	//waiting for ack with timeout
	upd_151_uart_rx(rxBuffer, &comWaitAck);
	retVal = upd_wait_rx_idle_timeout(TIMEOUT_DEFAULT);
	if (retVal == HAL_ERROR)
	{
		return HAL_ERROR;
	}









	return retVal;

}

