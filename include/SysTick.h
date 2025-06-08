#ifndef SYSTICK_REG
#define SYSTICK_REG
#include <sys_defs.h>

#define STCTRL *(volatile uint32_t*) 0xE000E010 // control register for the SysTick timer
#define STRELOAD *(volatile uint32_t*) 0xE000E014 // the reload value. this is how many clock cycles elapse before it is reset
#define STCURRENT *(volatile uint32_t*) 0xE000E018 // the value of the register currently. writes clear the register and the count status bit
#define COUNTMASK 0x00010000 // Access the 16th bit (count bit) of STCTRL
#define CYCLESPMS 16000 // 16,000,000 cycles per second * 0.001 seconds per millisecond
#define MAXCYCLES 0xFFFFFF // number of cycles we can represent with 23 bits
void delayByMs(uint32_t ms); // Block for this many milliseconds

#endif
