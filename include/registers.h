#ifndef REG_DEFS
#define REG_DEFS

#include <sys_defs.h>

// AHB enable register
#define SYSCTL_GPIOHBCTL_R *(volatile uint32_t*) 0x400FE06C 

// UART clock enable
#define RCGCUART_R *(volatile uint32_t *) 0x400FE618

// GPIO clock enable
#define RCGCGPIO_R *(volatile uint32_t *) 0x400FE608

// Digital enable register
#define GPIODEN_A_R *(volatile uint32_t *) 0x4005851C
#define GPIODEN_B_R *(volatile uint32_t *) 0x4005951C

// Clock source config register
#define UARTCC_A_R *(volatile uint32_t *) 0x4000CFC8
#define UARTCC_B_R *(volatile uint32_t *) 0x4000DFC8

// 2 mA drive select
#define GPIODR2R_A_R *(volatile uint32_t *) 0x40058500
#define GPIODR2R_B_R *(volatile uint32_t *) 0x40059500

// Alternate function select register
#define GPIOAFSEL_A_R *(volatile uint32_t *) 0x40058420
#define GPIOAFSEL_B_R *(volatile uint32_t *) 0x40059420
#define UART1_RX 0x1
#define UART1_TX 0x2

// GPIO port control register
#define GPIOPCTL_A_R *(volatile uint32_t *) 0x4005852C
#define GPIOPCTL_B_R *(volatile uint32_t *) 0x4005952C
#define PMC0_UART 0x01
#define PMC1_UART 0x10

// UART control register
#define UARTCTL_A_R *(volatile uint32_t *) 0x4000C030
#define UARTCTL_B_R *(volatile uint32_t *) 0x4000D030
#define RXE_B 0x200
#define TXE_B 0x100

// UART line control register
#define UARTLCRH_A_R *(volatile uint32_t *) 0x4000C02C
#define UARTLCRH_B_R *(volatile uint32_t *) 0x4000D02C
#define UART_FEN_EN 0x10
#define UART_FEN_DI ~(0x10)

// Baud rate generation
#define UARTIBRD_A_R *(volatile uint32_t *) 0x4000C024 // integer piece
#define UARTFBRD_A_R *(volatile uint32_t *) 0x4000C028 // fractional piece
#define UARTIBRD_B_R *(volatile uint32_t *) 0x4000D024 // integer piece
#define UARTFBRD_B_R *(volatile uint32_t *) 0x4000D028 // fraction piece

// UART Interrupt setup
#define UARTIM_B_R *(volatile uint32_t *) 0x4000D038 // UART1 Interrupt mask enable 
#define RXIM 0x20 // Receive interrupt mask
#define UARTIFLS_B_R *(volatile uint32_t *) 0x4000D034 // UART1 FIFO interrupt trigger level 
#define NVIC_EN0_R *(volatile uint32_t *) 0xE000E100 // NVIC EN0 register
#define UART1_RXEN 0x40 // UART1 RX interrupt
#define UARTICR_B_R *(volatile uint32_t *) 0x4000D044 // interrupt clear register

// FIFO registers
#define UARTFR_0_R *(volatile uint32_t *) 0x4000C018
#define UARTDR_0_R *(volatile uint32_t *) 0x4000C000
#define UARTFR_1_R *(volatile uint32_t *) 0x4000D018
#define UARTDR_1_R *(volatile uint32_t *) 0x4000D000

// Peripheral status registers
#define PRUART_R *(volatile uint32_t *) 0x400FEA18
#define PRGPIO_R *(volatile uint32_t *) 0x400FEA08

#define STCTRL *(volatile uint32_t*) 0xE000E010 // control register for the SysTick timer
#define STRELOAD *(volatile uint32_t*) 0xE000E014 // the reload value. this is how many clock cycles elapse before it is reset
#define STCURRENT *(volatile uint32_t*) 0xE000E018 // the value of the register currently. writes clear the register and the count status bit
#define COUNTMASK 0x00010000 // Access the 16th bit (count bit) of STCTRL
#define CYCLESPMS 16000 // 16,000,000 cycles per second * 0.001 seconds per millisecond
#define MAXCYCLES 0xFFFFFF // number of cycles we can represent with 23 bits

#endif