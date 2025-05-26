#include <stdint.h>
#include <stdbool.h>

// AHB enable register
#define SYSCTL_GPIOHBCTL_R *(volatile uint32_t*) 0x400FE06C 

// UART clock enable
#define RCGCUART_R *(volatile uint32_t *) 0x400FE618

// GPIO clock enable
#define RCGCGPIO_R *(volatile uint32_t *) 0x400FE608

// Digital enable register
#define GPIODEN_B_R *(volatile uint32_t *) 0x4005951C

// Clock source config register
#define UARTCC_B_R *(volatile uint32_t *) 0x4000DFC8

// 2 mA drive select
#define GPIODR2R_B_R *(volatile uint32_t *) 0x40059500

// Alternate function select register
#define GPIOAFSEL_B_R *(volatile uint32_t *) 0x40059420
#define UART1_RX 0x1
#define UART1_TX 0x2

// GPIO port control register
#define GPIOPCTL_B_R *(volatile uint32_t *) 0x4005952C
#define PMC0_UART 0x01
#define PMC1_UART 0x10

// UART control register
#define UARTCTL_B_R *(volatile uint32_t *) 0x4000D030
#define RXE_B 0x200
#define TXE_B 0x100

// UART line control register
#define UARTLCRH_B_R *(volatile uint32_t *) 0x4000D02C
#define UART_FEN_EN 0x10
#define UART_FEN_DI ~(0x10)

// Baud rate generation
#define UARTIBRD_B_R *(volatile uint32_t *) 0x4000D024 // integer piece
#define UARTFBRD_B_R *(volatile uint32_t *) 0x4000D028 // fraction piece

int main(void)
{
    // Enable UART module 1
    RCGCUART_R |= 0x2;

    // Enable clock for GPIO port B
    RCGCGPIO_R |= 0x2;

    // wait 3 cycles (quick and dirty)
    int i;
    for (i=0;i<3000;i++)
    {}

    // Enable AHB
    SYSCTL_GPIOHBCTL_R |= 0x20;

    // Enable digital function for pins 0 and 1 on GPIO port B
    GPIODEN_B_R |= 0x3;

    // Select alternate function for PB0 and PB1
    GPIOAFSEL_B_R |= UART1_RX;
    GPIOAFSEL_B_R |= UART1_TX;

    // Set pins 0 and 1 to 2mA drive
    GPIODR2R_B_R |= 0x3;

    // Select UART peripheral signal for PB0 and PB1
    GPIOPCTL_B_R &= ~0xFF;
    GPIOPCTL_B_R |= PMC0_UART | PMC1_UART;

    UARTCTL_B_R &= ~(0x1); // disable UART

    // flush FIFO in UARTLCRH
    UARTLCRH_B_R &= UART_FEN_DI;

    // Enable transmit and receive
    UARTCTL_B_R |= RXE_B;
    UARTCTL_B_R |= TXE_B;

    // Set the baud rate to 115200
    UARTLCRH_B_R &= ~(0x20); // Set ClkDiv to 16
    UARTIBRD_B_R = 8;
    UARTFBRD_B_R = 44;

    // Set UART clock to sys clock
    UARTCC_B_R = 0x0;

    UARTCTL_B_R |= UART_FEN_EN; // enable UART
}
