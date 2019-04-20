//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// UMSATS 2018-2020
//
// Repository:
//  UMSATS Google Drive: UMSATS/Guides and HowTos.../Command and Data Handling (CDH)/Coding Standards
//
// File Description:
//  Source file for external flash memory (W25N01GVxxIG) driver.
//	This driver is a modified version of the Mongoose OS Winbond W25XXX SPI NAND Flash Driver
//	https://github.com/mongoose-os-libs/vfs-dev-w25xxx
//
// History
// 2019-04-17 by Joseph Howarth
// - Created.
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
/*
 * Copyright (c) 2014-2018 Cesanta Software Limited
 * All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the ""License"");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ""AS IS"" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// INCLUDES
//-------------------------------------------------------------------------------------------------------------------------------------------------------------


#include "flash.h"

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// DEFINITIONS AND MACROS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// ENUMS AND ENUM TYPEDEFS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// STRUCTS AND STRUCT TYPEDEFS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// TYPEDEFS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// FUNCTION PROTOTYPES
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
//  This wrapper function is used to get the page number and offset for a given address.
//
// Parameters:
//
//	dd:			This should be a pointer to a FLASH_dev struct, which will be
//					used to refer to the device.
//
//	off:			The address for which the page number and page offset will be calculated.
//
//  page_num: 		A pointer to where the calculated page number will be stored.
//
//	page_off:		A pointer to where the calculated page offset will be stored.
//
// Returns:
//  Returns true if successful, false if not..
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
static bool flash_map_page(FlashDevice_t *dd, size_t off, uint16_t *page_num, uint16_t *page_off);

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
//  This function returns the minimum of two numbers.
//
// Parameters:
//
//	arg1:			The first number to compare.
//
//	arg2:			The second number to compare.
//
//  Returns:
//  The lower number.
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
static size_t MIN(size_t arg1, size_t arg2);

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
//  This function is used to get the page number and offset for a given address.
//
// Parameters:
//
//	dd:			This should be a pointer to a FLASH_dev struct, which will be
//					used to refer to the device.
//
//	off:			The address for which the page number and page offset will be calculated.
//
//  page_num: 		A pointer to where the calculated page number will be stored.
//
//	page_off:		A pointer to where the calculated page offset will be stored.
//
//  bb_reserve:		The number of reserved blocks.
//
// Returns:
//  Returns true if successful, false if not.
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
static bool flash_map_page_ex(FlashDevice_t *dd, size_t off, uint16_t *page_num,uint16_t *page_off, uint8_t bb_reserve);

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
//  This function loads one page of data into the flash memory buffer.
//
// Parameters:
//
//	dd:			This should be a pointer to a FLASH_dev struct, which will be
//				used to refer to the device.
//
//	page_num:	The page number that will be loaded into the flash memory buffer.
//
// Returns:
//  Enter description of return values (if any).
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
FlashStatus_t flash_page_data_read(FlashDevice_t *dd,uint16_t page_num);

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
//  This function sends a command along with an argument to the flash memory. It will store the response to the command.
//
// Parameters:
//
//	dd:			This should be a pointer to a FLASH_dev struct, which will be
//				used to refer to the device.
//
//	op:			The operation that should be executed by the flash.
//
//	arg_len:	The length of the argument for the command in bytes.
//
//	arg:		The argument for the command.
//
//	dummy_len:	The number of dummy bytes needed for the command.
//
//	rx_len:		Number of bytes to receive in response to command.
//
//	rx:			A pointer where the received bytes will be stored.
//
// Returns:
//  Enter description of return values (if any).
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool flash_op_arg_rx(FlashDevice_t *dd, FlashOperation_t op,size_t arg_len, uint32_t arg, int dummy_len,size_t rx_len, void *rx_data);

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
//  This function sends a command along with an argument to the flash memory.
//
// Parameters:
//
//	dd:			This should be a pointer to a FLASH_dev struct, which will be
//				used to refer to the device.
//
//	op:			The operation that should be executed by the flash.
//
//	arg_len:	The length of the argument for the command in bytes.
//
//	arg:		The argument for the command.
//
//
// Returns:
//  Enter description of return values (if any).
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
static bool flash_op_arg(FlashDevice_t *dd, FlashOperation_t op,size_t arg_len, uint32_t arg);

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// FUNCTIONS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool flash_map_page(FlashDevice_t *dd, size_t off, uint16_t *page_num, uint16_t *page_off) {

	return flash_map_page_ex(dd, off, page_num, page_off,dd->bb_reserve);

}

static size_t MIN(size_t arg1, size_t arg2){

	size_t ans;

	if(arg1<arg2){
		ans = arg1;
	}
	else if(arg2<arg1){
		ans = arg2;
	}
	else{
		ans = arg1;
	}
	return ans;
}

static bool flash_map_page_ex(FlashDevice_t *dd, size_t off,uint16_t *page_num,uint16_t *page_off, uint8_t bb_reserve) {

  bool res = false;
  size_t die_size = FLASH_DIE_SIZE;


  die_size -= ((size_t) bb_reserve) * FLASH_BLOCK_SIZE;
  if(off>die_size){
	  res = false;
  }
  else{
	  *page_num = off / FLASH_PAGE_SIZE;

	  if (page_off != NULL) *page_off = off % FLASH_PAGE_SIZE;

	  res = true;
  }

  return res;
}

static bool flash_op_arg_rx(FlashDevice_t *dd, FlashOperation_t op,size_t arg_len, uint32_t arg, int dummy_len,size_t rx_len, void *rx_data) {

	uint8_t buf[5] = {op};
  size_t tx_len;
  switch (arg_len) {
    case 0:
      tx_len = 1;
      break;
    case 1:
      buf[1] = (arg & 0xff);
      tx_len = 2;
      break;
    case 2:
      buf[1] = ((arg >> 8) & 0xff);
      buf[2] = (arg & 0xff);
      tx_len = 3;
      break;
    case 3:
      buf[1] = ((arg >> 16) & 0xff);
      buf[2] = ((arg >> 8) & 0xff);
      buf[3] = (arg & 0xff);
      tx_len = 4;
      break;
    default:
      return false;
  }
  return flash_txn(dd, tx_len, buf, dummy_len, rx_len, rx_data);
}

static bool flash_op_arg(FlashDevice_t *dd, FlashOperation_t op,size_t arg_len, uint32_t arg) {

	return w25xxx_op_arg_rx(dd, op, arg_len, arg, 0, 0, NULL);
}

FlashStatus_t flash_page_data_read(FlashDevice_t *dd,uint16_t page_num) {

	uint8_t st;
	/* Note: note selecting die, assuming already selected. */
	if (!flash_op_arg(dd, FLASH_OP_PAGE_DATA_READ, 1 + 2, page_num)) {
		return FLASH_ERROR;
	}

	while ((st = flash_read_reg(dd, FLASH_REG_STAT)) & FLASH_REG_STAT_BUSY) {
	}

	if (dd->ecc_chk) {

		st = flash_read_reg(dd, FLASH_REG_STAT);

		if (st & (FLASH_REG_STAT_ECC1 | FLASH_REG_STAT_ECC0)) {

			bool hard = (st & FLASH_REG_STAT_ECC1);

			if (hard){
				return FLASH_READ_ERROR_MULTI;
			}else{
				return FLASH_READ_ERROR_SINGLE;
			}
		}
	}

	return FLASH_OK;
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

FlashStatus_t flash_read(FlashDevice_t *dev, size_t address, size_t len, void *dst) {

  FlashStatus_t res = FLASH_ERROR;
  FlashStatus_t res2 = FLASH_ERROR;


  const size_t orig_len = len;

  uint8_t *dp = (uint8_t *) dst;


  uint16_t page_num, page_off;

  while (len > 0) {

    if (!flash_map_page(dev, address, &page_num, &page_off)) {
      res = FLASH_INVALID;
      goto out;
    }
    size_t rd_len = MIN(len, FLASH_PAGE_SIZE - page_off);

    //if (!w25mxx_select_die(dev, die_num)) goto out;
    if ((res2 = flash_page_data_read(dev, page_num))) {
      res = res2;
      goto out;
    }
    if (!flash_op_arg_rx(dev, FLASH_OP_READ, 2, page_off, 1, rd_len, dp)) {
      goto out;
    }
    address += rd_len;
    len -= rd_len;
    dp += rd_len;
  }
  res = FLASH_OK;
out:

  return res;
}



FlashStatus_t flash_dev_init(FlashDevice_t * dev,CoreSPIInstance_t spi, mss_gpio_id_t ss_pin, uint8_t bb_reserve, EccCheck_t ecc_check){

	FlashStatus_t result = FLASH_ERROR;

	dev->size = FLASH_DIE_SIZE;
	dev->ss_port_id = ss_pin;
	dev->spi = spi;
	dev->ecc_chk = ecc_check;
	dev->bb_reserve = bb_reserve;

	FlashOperation_t command = FLASH_OP_READ_JEDEC_ID;
	uint8_t id_buffer[3];
	spi_transaction_block_read_without_toggle(dev->spi,SPI_SLAVE_0,dev->ss_port_id,(uint8_t *)&command,1,id_buffer,3);

	if(id_buffer[0] == FLASH_ID_1 && id_buffer[1] == FLASH_ID_2 && id_buffer[2] == FLASH_ID_3){

		result = FLASH_OK;
	}

	return result;

}

