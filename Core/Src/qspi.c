/***************************************************************************************************************/
/* File:             rad_pcf2131.c                          Autor:           Hauer/Suprunenko                  */
/*                                                                                                             */
/* Changed by:       ---------                              Date:            --.--.----                        */
/*                                                                                                             */
/* Version:          1.000                                                                                     */
/*                                                                                                             */
/* Created on        22.02.2024                                                                                */
/*                                                                                                             */
/* Projekt:          Radon frontend                                                                            */
/*                                                                                                             */
/***************************************************************************************************************/
/* Description:      Routines for RTC chip rad_pcf2131.c                                                        */
/*                                                                                                             */
/***************************************************************************************************************/
/* History:                                                                                                    */
/*                                                                                                             */
/***************************************************************************************************************/
#include <string.h>
#include "qspi.h"
#include "stm32F7xx_hal.h"
#include "stm32f7xx.h"

extern QSPI_HandleTypeDef hqspi;




/*
 * Read the bank register
 */
uint8_t qspi_command_BRRD(uint8_t *reg)
{
	uint8_t retVal = HAL_OK;

	// wait until the busy flag go down
	while (hqspi.Instance->SR & QUADSPI_SR_BUSY){}
	config_qspi_indirect_read_1L();

	// 1 byte to read
	QUADSPI->DLR = 0;

	// write command in the instruction register
	MODIFY_REG(hqspi.Instance->CCR, QUADSPI_CCR_INSTRUCTION, READ_BANK_REG_CMD);

	//read the data
	*reg = hqspi.Instance->DR;

	return retVal;
}


/*
 * Write the bank register
 */
uint8_t qspi_command_BRWR(uint8_t *reg)
{
	uint8_t retVal = HAL_OK;
	while (hqspi.Instance->SR & QUADSPI_SR_BUSY){}
	config_qspi_indirect_write_1L();

	// 1 byte to write
	QUADSPI->DLR = 0;

	// write command in the instruction register
	MODIFY_REG(hqspi.Instance->CCR, QUADSPI_CCR_INSTRUCTION, WRITE_BANK_REG_CMD);

	hqspi.Instance->DR = *reg;


	return retVal;
}
uint8_t qspi_config()
{
	uint8_t retVal = HAL_OK;
	while (hqspi.Instance->SR & QUADSPI_SR_BUSY){}
	if (READ_REG(hqspi.Instance->SR) & QUADSPI_SR_BUSY)
	{
		return HAL_ERROR;
	}
	/*
	 * General configuration register
	 */
	// reset the configuration register
	hqspi.Instance->CR = 0x00;
	// prescaler 1:25
	//	MODIFY_REG(hqspi.Instance->CR, QUADSPI_CR_PRESCALER, (((uint8_t)25) << QUADSPI_CR_PRESCALER_Pos));
	MODIFY_REG(hqspi.Instance->CR, QUADSPI_CR_PRESCALER, (((uint8_t)1) << QUADSPI_CR_PRESCALER_Pos));	//Test 1:1
	// 1/2 sampling shift delay
	//MODIFY_REG(hqspi.Instance->CR, QUADSPI_CR_SSHIFT, QSPI_SAMPLE_SHIFTING_HALFCYCLE);
	MODIFY_REG(hqspi.Instance->CR, QUADSPI_CR_SSHIFT, QSPI_SAMPLE_SHIFTING_NONE);

	/*
	 * Ext memory parametre register
	 */
	// configuring flash size
	hqspi.Instance->DCR = 0x00;
	// 64MB = 2^(FSISE+1)
	MODIFY_REG(hqspi.Instance->DCR, QUADSPI_DCR_FSIZE, ((uint8_t)25) << QUADSPI_DCR_FSIZE_Pos);
	// clk high when idle
	MODIFY_REG(hqspi.Instance->DCR, QUADSPI_DCR_CKMODE, QUADSPI_DCR_CKMODE);


	// enable the QSPI
	SET_BIT(hqspi.Instance->CR, QUADSPI_CR_EN);

	// keep high the reset pin
	HAL_GPIO_WritePin(QSPI_RES_F13_GPIO_Port, QSPI_RES_F13_Pin, GPIO_SPEED_HIGH);
	return retVal;
}



//uint8_t config_qspi_indirect_write_1L()
//{
//	uint8_t retVal = HAL_OK;
//	while (hqspi.Instance->SR & QUADSPI_SR_BUSY){}
//	if (READ_REG(hqspi.Instance->SR) & QUADSPI_SR_BUSY)
//	{
//		return HAL_ERROR;
//	}
//
//	/*
//	 * General configuration register
//	 */
//	// reset the configuration register
//	hqspi.Instance->CR = 0x00;
//	// prescaler 1:25
//	MODIFY_REG(hqspi.Instance->CR, QUADSPI_CR_PRESCALER, (((uint8_t)25) << QUADSPI_CR_PRESCALER_Pos));
//	// 1/2 sampling shift delay
//	MODIFY_REG(hqspi.Instance->CR, QUADSPI_CR_SSHIFT, QSPI_SAMPLE_SHIFTING_NONE);
//
//
//	/*
//	 * Ext memory parametre register
//	 */
//	// configuring flash size
//	hqspi.Instance->DCR = 0x00;
//	// 64MB = 2^(FSISE+1)
//	MODIFY_REG(hqspi.Instance->DCR, QUADSPI_DCR_FSIZE, ((uint8_t)25) << QUADSPI_DCR_FSIZE_Pos);
//
//	// enable the QSPI
//	SET_BIT(hqspi.Instance->CR, QUADSPI_CR_EN);
//}




/*
 *  Write into the control register of the ext flash
 *  first byte is the status register 1
 *  second byte is the control register
 */
unsigned int qspi_command_WRR(uint8_t* buffer)
{
	QSPI_CommandTypeDef      sCommand = {};

	sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	sCommand.Instruction = WRITE_STATUS_REG_CMD;
	sCommand.AddressMode = QSPI_ADDRESS_NONE;
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DataMode = QSPI_DATA_1_LINE;
	sCommand.DummyCycles = 0;
	sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
	sCommand.NbData = 2;


	if (QSPI_WriteEnable() != HAL_OK) {
		return HAL_ERROR;
	}

	if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
		return HAL_ERROR;
	}

	if (HAL_QSPI_Transmit(&hqspi, buffer, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
		return HAL_ERROR;
	}
	if (QSPI_AutoPollingMemReady() != HAL_OK) {
		return HAL_ERROR;
	}

	return HAL_OK;
}


uint8_t QSPI_AutoPollingMemReady(void) {
	QSPI_CommandTypeDef sCommand;
	QSPI_AutoPollingTypeDef sConfig;
	memset(&sCommand, 0, sizeof(sCommand));
	memset(&sConfig, 0, sizeof(sConfig));
	/* Configure automatic polling mode to wait for memory ready ------
	 */
	sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	sCommand.Instruction = READ_STATUS_REG_CMD;
	sCommand.AddressMode = QSPI_ADDRESS_NONE;
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DataMode = QSPI_DATA_1_LINE;
	sCommand.DummyCycles = 0;
	sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
	sConfig.Match = 0x00;
	sConfig.Mask = 0x01;
	sConfig.MatchMode = QSPI_MATCH_MODE_AND;
	sConfig.StatusBytesSize = 1;
	sConfig.Interval = 0x10;
	sConfig.AutomaticStop = QSPI_AUTOMATIC_STOP_ENABLE;
	if (HAL_QSPI_AutoPolling(&hqspi, &sCommand, &sConfig,
			HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
		return HAL_ERROR;
	}
	return HAL_OK;
}

unsigned int rad_qspi_disable_mem_map_mode()
{
	uint8_t retVal = HAL_OK;
	// return in indirect mode
	retVal = HAL_QSPI_Abort(&hqspi);
	qspi_config();
	return retVal;
}

unsigned int  rad_qspi_flash_enable_mem_map_mode (unsigned int Mode)
{
	QSPI_CommandTypeDef      s_command  = {};
	QSPI_MemoryMappedTypeDef s_mem_mapped_cfg;

	switch(Mode)
	{
	case SPI_1I2O_MODE :           /* 1-1-2 read commands */
		s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
		s_command.Instruction     = DUAL_OUT_FAST_READ_4_BYTE_ADDR_CMD;
		s_command.AddressMode     = QSPI_ADDRESS_1_LINE;
		s_command.DataMode        = QSPI_DATA_2_LINES;
		s_command.DummyCycles     = DUMMY_CYCLES_READ;
		break;

	case SPI_2IO_MODE :           /* 1-2-2 read commands */
		s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
		s_command.Instruction     = DUAL_INOUT_FAST_READ_4_BYTE_ADDR_CMD;
		s_command.AddressMode     = QSPI_ADDRESS_2_LINES;
		s_command.DataMode        = QSPI_DATA_2_LINES;
		s_command.DummyCycles     = DUMMY_CYCLES_READ_DUAL_INOUT;
		break;

	case SPI_1I4O_MODE :           /* 1-1-4 read commands */
		s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
		s_command.Instruction     = QUAD_OUT_FAST_READ_4_BYTE_ADDR_CMD;
		s_command.AddressMode     = QSPI_ADDRESS_1_LINE;
		s_command.DataMode        = QSPI_DATA_4_LINES;
		s_command.DummyCycles     = DUMMY_CYCLES_READ;
		break;

	case SPI_4IO_MODE :           /* 1-4-4 read commands */
		s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
		s_command.Instruction     = QUAD_INOUT_FAST_READ_4_BYTE_ADDR_CMD;
		s_command.AddressMode     = QSPI_ADDRESS_4_LINES;
		s_command.DataMode        = QSPI_DATA_4_LINES;
		s_command.DummyCycles     = DUMMY_CYCLES_READ_QUAD_INOUT;
		break;

	case SPI_MODE :               /* 1-1-1 commands, Power on H/W default setting */
	default:
		s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
		s_command.Instruction     = FAST_READ_4_BYTE_ADDR_CMD;
		s_command.AddressMode     = QSPI_ADDRESS_1_LINE;
		s_command.DataMode        = QSPI_DATA_1_LINE;
		s_command.DummyCycles     = 2;
		s_command.DummyCycles     = 8;
		break;
	}

	//s_command.DummyCycles        = 6;
	s_command.AlternateByteMode  = QSPI_ALTERNATE_BYTES_NONE;

	s_command.AlternateBytesSize = QSPI_ALTERNATE_BYTES_NONE;
	s_command.AlternateBytes =QSPI_ALTERNATE_BYTES_NONE;

	s_command.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;
	/* Configure the command for the read instruction */
	s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.AddressSize        = QSPI_ADDRESS_32_BITS;
	s_command.DdrMode            = QSPI_DDR_MODE_DISABLE;
	/* Configure the memory mapped mode */
	s_mem_mapped_cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
	s_mem_mapped_cfg.TimeOutPeriod     = 0x00;

	if (HAL_QSPI_MemoryMapped(&hqspi, &s_command, &s_mem_mapped_cfg) != HAL_OK)
	{
		return HAL_ERROR;
	}

	return HAL_OK;
}

unsigned int rad_qspi_flash_read_status_reg(uint8_t* buffer)
{
	QSPI_CommandTypeDef      sCommand = {};

	sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	sCommand.Instruction = READ_STATUS_REG_CMD;
	sCommand.AddressMode = QSPI_ADDRESS_NONE;
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DataMode = QSPI_DATA_1_LINE;
	sCommand.DummyCycles = 0;
	sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
	sCommand.NbData = 1;

	if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
		return HAL_ERROR;
	}
	if (HAL_QSPI_Receive(&hqspi, buffer, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
		return HAL_ERROR;
	}

	return HAL_OK;

}

unsigned int rad_qspi_flash_read_control_reg(uint8_t* buffer)
{

	QSPI_CommandTypeDef      sCommand = {};


	sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	sCommand.Instruction = READ_CONFIGURATION_REG_CMD;
	sCommand.AddressMode = QSPI_ADDRESS_NONE;
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DataMode = QSPI_DATA_1_LINE;
	sCommand.DummyCycles = 0;
	sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
	sCommand.NbData = 1;

	if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
		return HAL_ERROR;
	}
	if (HAL_QSPI_Receive(&hqspi, buffer, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
		return HAL_ERROR;
	}

	return HAL_OK;

}



uint8_t CSP_QSPI_WriteMemory(uint8_t* buffer, uint32_t address, uint32_t buffer_size)
{
	QSPI_CommandTypeDef sCommand;
	uint32_t end_addr, current_size, current_addr;

	/* Calculation of the size between the write address and the end of the page */
	current_addr = 0;

	//
	while (current_addr <= address)
	{
		current_addr += MEMORY_PAGE_SIZE;
	}
	current_size = current_addr - address;

	/* Check if the size of the data is less than the remaining place in the page */
	if (current_size > buffer_size) {
		current_size = buffer_size;
	}

	/* Initialize the adress variables */
	current_addr = address;
	end_addr = address + buffer_size;

	sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	sCommand.AddressSize = QSPI_ADDRESS_32_BITS;
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_NONE;
	sCommand.AlternateBytes =QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode = QSPI_SIOO_INST_ONLY_FIRST_CMD;
	sCommand.Instruction = WRITE_PAGE_CMD;

	sCommand.AddressMode = QSPI_ADDRESS_1_LINE;

	sCommand.DataMode = QSPI_DATA_1_LINE;
	sCommand.NbData = buffer_size;
	sCommand.Address = address;
	sCommand.DummyCycles = 0;

	/* Perform the write page by page */
	do {
		sCommand.Address = current_addr;
		sCommand.NbData = current_size;

		if (current_size == 0) {
			return HAL_OK;
		}

		/* Enable write operations */
		if (QSPI_WriteEnable() != HAL_OK) {
			return HAL_ERROR;
		}

		/* Configure the command */
		if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
			return HAL_ERROR;
		}
		MODIFY_REG(hqspi.Instance->CR, QUADSPI_CR_SSHIFT, QSPI_SAMPLE_SHIFTING_NONE);


		/* Transmission of the data */
		//		if (HAL_QSPI_Transmit(&hqspi, buffer, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
		//			return HAL_ERROR;
		//		}

		if (HAL_QSPI_Transmit_DMA(&hqspi, buffer) != HAL_OK) {
			return HAL_ERROR;
		}

		/* Configure automatic polling mode to wait for end of program */
		if (QSPI_AutoPollingMemReady() != HAL_OK) {
			return HAL_ERROR;
		}

		/* Update the address and size variables for next page programming */
		current_addr += current_size;
		buffer += current_size;
		current_size = ((current_addr + MEMORY_PAGE_SIZE) > end_addr) ? (end_addr - current_addr) : MEMORY_PAGE_SIZE;
	} while (current_addr <= end_addr);

	return HAL_OK;
}

uint8_t
CSP_QSPI_ReadMemory(uint8_t* buffer, uint32_t address, uint32_t buffer_size) {

	QSPI_CommandTypeDef sCommand;
	uint32_t end_addr, current_size, current_addr;

	/* Calculation of the size between the read address and the end of the page */
	current_addr = 0;

	//
	while (current_addr <= address) {
		current_addr += MEMORY_PAGE_SIZE;
	}
	current_size = current_addr - address;

	/* Check if the size of the data is less than the remaining place in the page */
	if (current_size > buffer_size) {
		current_size = buffer_size;
	}

	/* Initialize the adress variables */
	current_addr = address;
	end_addr = address + buffer_size;

	sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	sCommand.AddressSize = QSPI_ADDRESS_32_BITS;
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_NONE;
	sCommand.AlternateBytes =QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	//sCommand.SIOOMode = QSPI_SIOO_INST_ONLY_FIRST_CMD; // do not send the instruction on every transaction
	sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD; // do not send the instruction on every transaction
	sCommand.Instruction = READ_4_BYTE_ADDR_CMD;
	sCommand.AddressMode = QSPI_ADDRESS_1_LINE;

	sCommand.DummyCycles = 0;		// no dummy cycles

	//	sCommand.DataMode = QSPI_DATA_NONE;
	sCommand.DataMode = QSPI_DATA_1_LINE;

	sCommand.NbData = buffer_size;
	sCommand.Address = address;
	//sCommand.DummyCycles = 6;

	/* Perform a read page by page */
	do {
		sCommand.Address = current_addr;
		sCommand.NbData = current_size;

		if (current_size == 0) {
			return HAL_OK;
		}

		/* Configure the command */
		if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
			return HAL_ERROR;
		}

		//while (HAL_QSPI_GetState (&hqspi)!=HAL_QSPI_STATE_READY); //debug

		/* Transmission of the data */
		//		if (HAL_QSPI_Receive(&hqspi, buffer, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
		//			return HAL_ERROR;
		//		}
		if (HAL_QSPI_Receive_DMA(&hqspi, buffer) != HAL_OK) {
			return HAL_ERROR;
		}


		/* Update the address and size variables for next page programming */
		current_addr += current_size;
		buffer += current_size;
		current_size = ((current_addr + MEMORY_PAGE_SIZE) > end_addr) ? (end_addr - current_addr) : MEMORY_PAGE_SIZE;
	} while (current_addr <= end_addr);

	return HAL_OK;
}

uint8_t CSP_QSPI_EraseSector(uint32_t EraseStartAddress, uint32_t EraseEndAddress)
{
	QSPI_CommandTypeDef sCommand;

	EraseStartAddress = EraseStartAddress
			- EraseStartAddress % MEMORY_SECTOR_SIZE;

	/* Erasing Sequence -------------------------------------------------- */
	sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	sCommand.AddressSize = QSPI_ADDRESS_32_BITS;
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
	sCommand.Instruction = SECTOR_ERASE_CMD;
	sCommand.AddressMode = QSPI_ADDRESS_1_LINE;

	sCommand.DataMode = QSPI_DATA_NONE;
	sCommand.DummyCycles = 0;

	while (EraseEndAddress >= EraseStartAddress)
	{
		sCommand.Address = (EraseStartAddress & 0x03FFFFFF);
		if (QSPI_WriteEnable() != HAL_OK)
		{
			return HAL_ERROR;
		}
		if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		{
			return HAL_ERROR;
		}
		EraseStartAddress += MEMORY_SECTOR_SIZE;
		if (QSPI_AutoPollingMemReady() != HAL_OK)
		{
			return HAL_ERROR;
		}
	}
	return HAL_OK;
}

uint8_t QSPI_WriteEnable(void)
{
	QSPI_CommandTypeDef sCommand;
	QSPI_AutoPollingTypeDef sConfig;

	/* Enable write operations ------------------------------------------ */
	sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	sCommand.Instruction = WRITE_ENABLE_CMD;
	sCommand.AddressMode = QSPI_ADDRESS_NONE;
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DataMode = QSPI_DATA_NONE;
	sCommand.DummyCycles = 0;
	sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
		return HAL_ERROR;
	}

	/* Configure automatic polling mode to wait for write enabling ---- */
	sConfig.Match = 0x02;
	sConfig.Mask = 0x02;
	sConfig.MatchMode = QSPI_MATCH_MODE_AND;
	sConfig.StatusBytesSize = 1;
	sConfig.Interval = 0x10;
	sConfig.AutomaticStop = QSPI_AUTOMATIC_STOP_ENABLE;

	sCommand.Instruction = READ_STATUS_REG_CMD;
	sCommand.DataMode = QSPI_DATA_1_LINE;
	if (HAL_QSPI_AutoPolling(&hqspi, &sCommand, &sConfig, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
		return HAL_ERROR;
	}

	return HAL_OK;
}


