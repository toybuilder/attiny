/*
 * nvm.c
 *
 * Created: 10/26/2019 12:49:52 PM
 *  Author: Joseph
 *  NVM Routines per https://www.avrfreaks.net/forum/attiny104-self-programming
 *  Additional bits added to make it easier to understand and reuse.
 */ 

#include "nvm.h"


// Per ATTiny104 Datasheet
// Code (program memory)
// Section Size 1024 (Bytes)
// Page Size	   8 (Words)
// Pages		  64
// WADDR		[3:1]
// PADDR		[9:4]
// 8 words * 64 pages = 512 words = 1024 bytes
//
// 20.4.1. Addressing the Flash
//   The data space uses byte accessing but since the Flash sections are accessed as words and organized
//   in pages, the byte-address of the data space must be converted to the word-address of the Flash section.
//   The most significant bits of the data space address select the NVM Lock bits or the Flash section mapped
//   to the data memory. The word address within a page (WADDR) is held by bits [WADDRMSB:1], and the
//   page address (PADDR) by bits [PADDRMSB:WADDRMSB+1]. Together, PADDR and WADDR form the
//   absolute address of a word in the Flash section.
//   The least significant bit of the Flash section address is used to select the low or high byte of the word.
//
// Memory map
//   Memory Configuration
//
//   Name             Origin             Length             Attributes
//   text             0x00000000         0x00001000         xr
//   data             0x00800040         0x00000100         rw !x
//   config           0x00820000         0x00000001         rw !x
//   lock             0x00830000         0x00000002         rw !x
//   signature        0x00840000         0x00000004         rw !x
//   *default*        0x00000000         0xffffffff
// NVM Routines per https://www.avrfreaks.net/forum/attiny104-self-programming

// https://www.avrfreaks.net/forum/declare-progmem-variable-certain-address?page=all

// See important data regarding memory space in ATTiny104 databook, chapter 9.

//static __attribute__((used, section(".nvmstore"))) const uint16_t nvmword = 0x1234;


void PAGE_Erase(uint16_t address)
{
	// NOTE: Page erase must make a dummy write to the high-byte address of a word inside the page to be erased
	asm volatile (
	"ldi  r19,0x18  \n\t"
	"out  0x33,r19  \n\t"   /* set NVMCMD to PAGE_ERASE (0b011000 per section 20.5); */
	"ldi  r19,0xE7  \n\t"
	"out  0x3C,r19  \n\t"   /* CCP = 0xE7 NVM self-programming enable */
	/* trigger page erase within four clock cycles.*/
	"ldi  r22,0x00  \n\t"
	"st  Z+,r22     \n\t"
	"nop            \n\t"  /* two nops are required after the ST operation that triggers the erase operation */
	"nop  "
	:                       /* No outputs. */
	:"z" (address)
	:"r19", "r22"
	);
	while (NVMCSR & (1 << NVMBSY) );  // Wait while memory is being programmed
}

void NVM_Write(uint16_t address, uint16_t word) // Stores MSB first then LSB
{
	uint8_t hbyte = word>>8;
	asm volatile (
	"ldi r19,0x1D    \n\t"
	"out 0x33,r19    \n\t"  /* set NVMCMD to WORD_WRITE; */
	"ldi r19,0xE7    \n\t"
	"out 0x3C,r19    \n\t"  /* CPP = 0xE7 NVM self-programming enable */
	/* trigger value write within four clock cycles.*/
	"st Z+,%1        \n\t"
	"st Z,%2         \n\t"
	"nop             \n\t"
	"nop "
	:                       /* No outputs. */
	:"e" (address),"r"(word),"r"(hbyte)
	:"r19"
	);
	while (NVMCSR & (1 << NVMBSY) );  // Wait while NVM memory is busy, being programmed.
}

void MEM_Write(uint16_t address, uint8_t value)
{
	asm volatile (
	"st Z, %1        \n\t"
	:   /* No outputs. */
	:"z" (address),"r"(value)
	);
}

uint8_t MEM_Read(uint16_t address)
{
	uint8_t value;
	asm volatile (
	"ld  %0,Z       \n\t"
	:"=r" (value)
	:"z" (address)
	);
	return value;
}

void  __attribute__ ((weak)) blinkout(uint8_t value) {
	// Do something here to blink LEDs or otherwise send the value out	
	// Define your own somewhere else, or modify locally
}

void NVM_ShowValue(uint16_t address) {
	// Read value from address and write it out, digit by digit
	uint16_t value;
	value=MEM_Read(address) | MEM_Read(address+1)<<8;	
	blinkout((value>>12)&0xf);
	blinkout((value>>8)&0xf);
	blinkout((value>>4)&0xf);
	blinkout(value&0xf);	
}

void NVM_EraseWriteAndShow (uint16_t address, uint16_t value) {
	// NOTE: address must be word-aligned
	PAGE_Erase(address+1);
	NVM_Write(address,value);
	NVM_ShowValue(address);	
}

void NVM_test(void) {
	// Each bracketed chunk of test code demonstrates important concepts on the use of the NVM self-programming features
	
	#if 0
	{
		// If this is the first run after programming, the flash will have been erased, showing 0xffff
		// If you reset and restart the processor after a write, it will show the currently stored value
		NVM_ShowValue(0x43e0);
	}

	{
		// This should work as expected, showing 0x7310
		NVM_EraseWriteAndShow(0x43e0, 0x7310);  
	}
	
	{
		// this should work as expected, showing 0x1111 and 0x2222
		NVM_EraseWriteAndShow(0x43e0, 0x1111);  
		NVM_EraseWriteAndShow(0x43e0, 0x2222);		
	}
	
	{
		// this demonstrates that we can write a 0 over a 1, but not a 1 over a 0
		NVM_EraseWriteAndShow(0x43e0, 0x3300);  // This shows 0x3300
		NVM_Write(0x43e0, 0x1111);  // If we write without erasing first, we can't write a 1 over already-burned 0
		NVM_ShowValue(0x43e0); // Since (0x3300) & (0x1111) == 0x1100, this results in showing 0x1100 (instead of 0x1111)		
	}

#endif
	{
		// This shows that the page erase does not work if the low-offset byte in a word is selected for page erase
		NVM_EraseWriteAndShow(0x43e0, 0x1111);		
		PAGE_Erase(0x43e0);    
		NVM_ShowValue(0x43e0);
	}

	{
		// This works.  We must use a high byte address for the PAGE_Erase command
		NVM_EraseWriteAndShow(0x43e0, 0x2222);
		PAGE_Erase(0x43e1);    
		NVM_ShowValue(0x43e0);
	}
	
	{
		// This also works.			
		NVM_EraseWriteAndShow(0x43e0, 0x3333);
		PAGE_Erase(0x43e3);
		NVM_ShowValue(0x43e0);
	}
	
	while(1) // HALT
	  ;
	// After initial programming via TPI, we should get 0xFFFF (15 blinks, 4 times)
	// After power down and restart, we should see 0x1234 (1 2 3 and 4 blinks)
	
}
