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


extern QSPI_HandleTypeDef hqspi;




unsigned char rad_qspi_flash_write_enable(void) {
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


/***************************************************************************************************************/
/*  NAME:         uint8_t QSPI_Configuration(void)                                                             */
/*-------------------------------------------------------------------------------------------------------------*/
/*  PURPOSE:      In initializes S25FL512S QSPI FLASH memory                                                   */
/*-------------------------------------------------------------------------------------------------------------*/
/*  PARAMETER:    none                                                                                         */
/*-------------------------------------------------------------------------------------------------------------*/
/*  RETURN:       The result of the operation   (HAL_OK or HAL_ERROR)                                          */
/*-------------------------------------------------------------------------------------------------------------*/
/*  DESCRIPTION:                                                                                               */
/*-------------------------------------------------------------------------------------------------------------*/
/*  HISTORY:      27.04.2022 Creation, Peter Hauer, Bertin GmbHH                                               */
/***************************************************************************************************************/
unsigned char  rad_qspi_flash_init(void)
{
	QSPI_CommandTypeDef sCommand;
	uint8_t test_buffer[4] = { 0 };
	/****************************************** read status register **********************************/
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
	if (HAL_QSPI_Receive(&hqspi, test_buffer, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
		return HAL_ERROR;
	}
	/****************************** read configuration register *****************************************/
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
	if (HAL_QSPI_Receive(&hqspi, &(test_buffer[1]), HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
		return HAL_ERROR;
	}
	/************************** set 4bit mode and latency ***********************************************/
	test_buffer[0] |= 0x00;         /* Status register: modify buffer to enable quad mode */
	test_buffer[1]  = 0x02;         /* Config register: set dummy cycles and enable QUAD mode  */

	/******************************* write configuration regiset ****************************************/
	//    test_buffer[0] |= 0x40;         /* modify buffer to enable quad mode */
	//    test_buffer[1] |= 0xC0;         /* set dummy cycles */

	sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	sCommand.AddressSize = QSPI_ADDRESS_32_BITS;
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
	sCommand.Instruction = WRITE_STATUS_REG_CMD;
	sCommand.AddressMode = QSPI_ADDRESS_NONE;
	sCommand.DataMode = QSPI_DATA_1_LINE;
	sCommand.DummyCycles = 0;
	sCommand.NbData = 2;

	if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
		return HAL_ERROR;
	}
	if (HAL_QSPI_Transmit(&hqspi, test_buffer, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
		Error_Handler();
		return HAL_ERROR;
	}

	/********************** the read bank register command ***************************************/
	sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
	sCommand.Instruction       = READ_BANK_REG_CMD;
	sCommand.AddressMode       = QSPI_ADDRESS_NONE;
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DataMode          = QSPI_DATA_1_LINE;
	sCommand.DummyCycles       = 0;
	sCommand.NbData            = 1;
	sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
	sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	/* Configure the command */
	if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return HAL_ERROR;
	}

	/* Reception of the data */
	if (HAL_QSPI_Receive(&hqspi,test_buffer, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return HAL_ERROR;
	}

	/* Enable write operations */
	if(rad_qspi_flash_write_enable()!=HAL_OK)
	{
		return HAL_ERROR;
	}

	/* Update Bank address register (with 4byte addressing bit) */
	sCommand.Instruction = WRITE_BANK_REG_CMD;
	test_buffer[0] = 0x80;                // set 4 byte addressing

	/* Configure the write Bank register configuration register command */
	if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return HAL_ERROR;
	}

	/* Transmission of the data Status Register 1 */
	if (HAL_QSPI_Transmit(&hqspi, test_buffer, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return HAL_ERROR;
	}



	sCommand.Instruction       = READ_BANK_REG_CMD;
	if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return HAL_ERROR;
	}

	/* Reception of the data */
	if (HAL_QSPI_Receive(&hqspi,test_buffer, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return HAL_ERROR;
	}

	return HAL_OK;
}




unsigned int  rad_qspi_flash_enable_mem_map_mode (unsigned int Mode)
{
	QSPI_CommandTypeDef      s_command;
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
		s_command.DummyCycles     = DUMMY_CYCLES_READ;
		break;
	}

	s_command.DummyCycles        = 6;
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


unsigned int rad_qspi_flash_read(uint32_t addr, uint8_t* buffer, uint32_t szt)
{
	QSPI_CommandTypeDef      s_command;

	s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction     = QUAD_INOUT_FAST_READ_4_BYTE_ADDR_CMD;
	s_command.AddressMode     = QSPI_ADDRESS_4_LINES;
	s_command.DataMode        = QSPI_DATA_4_LINES;
	s_command.DummyCycles     = DUMMY_CYCLES_READ_QUAD_INOUT;

	s_command.Address		  = addr;
	s_command.NbData		  = szt;

	if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return HAL_ERROR;
	}

	if (HAL_QSPI_Receive(&hqspi, buffer, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return HAL_ERROR;
	}




	return HAL_OK;
}
