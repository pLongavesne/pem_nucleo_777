/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    quadspi.h
  * @brief   This file contains all the function prototypes for
  *          the quadspi.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __QUADSPI_H__
#define __QUADSPI_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern QSPI_HandleTypeDef hqspi;

/* USER CODE BEGIN Private defines */

unsigned char rad_qspi_flash_write_enable(void);
unsigned char  rad_qspi_flash_init(void);
unsigned int  rad_qspi_flash_enable_mem_map_mode (unsigned int Mode);

unsigned int rad_qspi_flash_read(uint32_t addr, uint8_t* buffer, uint32_t szt);
unsigned int rad_qspi_flash_write_page(uint32_t addr, uint8_t* buffer, uint32_t szt);
unsigned int rad_qspi_flash_read_status_reg(uint8_t* buffer);
unsigned int rad_qspi_flash_read_control_reg(uint8_t* buffer);
unsigned int rad_qspi_flash_erase(uint32_t addr);

/* USER CODE END Private defines */



/* USER CODE BEGIN Prototypes */
typedef enum
{
  SPI_MODE = 0,                 /*!< 1-1-1 commands, Power on H/W default setting */
  SPI_1I2O_MODE,                /*!< 1-1-2 read commands                          */
  SPI_2IO_MODE,                 /*!< 1-2-2 read commands                          */
  SPI_1I4O_MODE,                /*!< 1-1-4 read commands                          */
  SPI_4IO_MODE,                 /*!< 1-4-4 read commands                          */
  DPI_MODE,                     /*!< 2-2-2 commands                               */
  QPI_MODE                      /*!< 4-4-4 commands                               */
} S25FL512S_Interface_t;


/*S25FL512 memory parameters*/
#define MEMORY_FLASH_SIZE               0x4000000 /* 512 MBits => 64MBytes */
#define MEMORY_BLOCK_SIZE               0x40000   /* 256 sectors of block size */
#define MEMORY_SECTOR_SIZE              0x40000   /* 2564 subsectors of 256 kBytes */
#define MEMORY_PAGE_SIZE                0x200     /* 131072 pages of 256 bytes */


/*MX25L512 commands */
#define WRITE_ENABLE_CMD             0x06    // OK Enable write access
#define READ_STATUS_REG_CMD          0x05    // OK Read status byte
#define READ_STATUS_2_REG_CMD          0x07    // OK Read status byte

#define WRITE_STATUS_REG_CMD         0x01    // OK Write status byte
//#define SECTOR_ERASE_CMD             0x20    // No sector erase
#define SECTOR_ERASE_CMD             0xDC    // Erase a sector/block
#define CHIP_ERASE_CMD               0xC7    // Erase entire chip
//#define QUAD_IN_FAST_PROG_CMD 0x38 (3 byte Address)
#define QUAD_IN_FAST_PROG_CMD        0x34    // Write 4Bit, 3byte Address

//#define READ_CONFIGURATION_REG_CMD   0x15
#define READ_CONFIGURATION_REG_CMD   0x35    // Read Configuration register
//#define QUAD_READ_IO_CMD             0xEC    // Read 4Bit, 4byte Address

// #define RESET_ENABLE_CMD 0x66
#define RESET_EXECUTE_CMD            0xF0    // software reset of the chip

#define READ_BANK_REG_CMD            0x16
#define WRITE_BANK_REG_CMD           0x17
#define ACCESS_BANK_REG_CMD          0xB9

#define READ_CMD                              0x03
#define READ_4_BYTE_ADDR_CMD                  0x13
#define FAST_READ_CMD                         0x0B
#define FAST_READ_4_BYTE_ADDR_CMD             0x0C
#define FAST_READ_DDR_CMD                     0x0D
#define FAST_READ_DDR_4_BYTE_ADDR_CMD         0x0E
#define DUAL_OUT_FAST_READ_CMD                0x3B
#define DUAL_OUT_FAST_READ_4_BYTE_ADDR_CMD    0x3C
#define QUAD_OUT_FAST_READ_CMD                0x6B
#define QUAD_OUT_FAST_READ_4_BYTE_ADDR_CMD    0x6C
#define DUAL_INOUT_FAST_READ_CMD              0xBB
#define DUAL_INOUT_FAST_READ_DTR_CMD          0xBD
#define DUAL_INOUT_FAST_READ_4_BYTE_ADDR_CMD  0xBC
#define DDR_DUAL_INOUT_READ_4_BYTE_ADDR_CMD   0xBE
#define QUAD_INOUT_FAST_READ_CMD              0xEB
#define QUAD_INOUT_FAST_READ_4_BYTE_ADDR_CMD  0xEC
#define QUAD_INOUT_FAST_READ_DDR_CMD          0xED
#define QUAD_INOUT_READ_DDR_4_BYTE_ADDR_CMD   0xEE

/* Dummy cycles for STR read mode */
#define DUMMY_CLOCK_CYCLES_READ_QUAD_LDR  3   //
#define DUMMY_CYCLES_READ_QUAD       8U
#define DUMMY_CYCLES_READ            8U
#define DUMMY_CYCLES_READ_DUAL_INOUT 4U
#define DUMMY_CYCLES_READ_QUAD_INOUT 6U


/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __QUADSPI_H__ */

