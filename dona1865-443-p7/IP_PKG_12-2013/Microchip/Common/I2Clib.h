/* 
** I2C Library
** Richard Wall
** October 3, 2011
**
*/

#ifndef __I2CLIB
	#define	__I2CLIB

// Configuration Bit settings
// System Clock = 80 MHz, Peripherial Bus = 10MHz
// Primary Osc w/PLL (XT+,HS+,EC+PLL)
// Input Divider 2x Divider
// Multiplier 20x Multiplier
//

	#define CCLK	(80000000) 	//8Mhz Osc on Explorer16 board (pll 8 / 2 * 15)
	#define PBCLK 	(CCLK/8)
	#define Fsck	357000		// This clock rate must be less than 400000
	#define BRG_VAL (PBCLK/2/Fsck)
// #define Nop() asm( "nop" )	


// Function Prototypes
	void initI2C(void);
	void i2c_wait(unsigned int cnt);

	int I2CWrtiteEEPROM(int SlaveAddrress, int mem_addr, char *data, int len);
	int I2CReadEEPROM(int SlaveAddrress, int mem_addr, char *data, int len);
	int I2CReadByte(int SlaveAddrress, char *data);
	int I2CWriteByte(int SlaveAddrress, char data);
	int wait_i2c_xfer(int SlaveAddress);

#endif

