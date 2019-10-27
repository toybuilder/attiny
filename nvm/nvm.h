/*
 * nvm.h
 *
 * Created: 10/26/2019 12:50:25 PM
 *  Author: Joseph Chiu
 *  NVM Routines per https://www.avrfreaks.net/forum/attiny104-self-programming
 *  Additional bits added to make it easier to understand and reuse.
 */ 


#ifndef NVM_H_
#define NVM_H_

#include <avr/io.h>

void PAGE_Erase(uint16_t address);
void NVM_Write(uint16_t address, uint16_t word);
void MEM_Write(uint16_t address, uint8_t value);
uint8_t MEM_Read(uint16_t address);

#ifdef NVM_DISABLE_FUSE_WARNING
// Do nothing
#else
#warning Reminder: you must set the fuse on the device to enable self programming (SELFPROGEN)
#endif

// ATTiny104 Flash page size is 8 words long = 16 bytes
// ATTiny104 datasheet, 9.3: The mapped program memory begins at byte address 0x4000 in data memory.
#define FLASH_PAGE_SIZE   (16) 
#define MEMMAP_FLASH_START (0x4000) 

//

#define NVM_BLK_LEN (1 * FLASH_PAGE_SIZE)
#define NVM_BLK_BASE ((FLASHEND+1)-NVM_BLK_LEN)
#define NVM_BLK_ADDR(offset) (MEMMAP_FLASH_START + NVM_BLK_BASE + offset)

#endif /* NVM_H_ */