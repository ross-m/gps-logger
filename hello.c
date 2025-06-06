#include <SysTick.h>
#include <string.h>

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

void UART1_handler(void)
{
        uint8_t junk_byte;

        while (!(UARTFR_1_R & 0x10))
        {
            junk_byte = UARTDR_1_R; // clear the input buffer
        } 

        // drop sentence in byte by byte
        char* hello = "hello world!\r\n\0";
        
        while (*hello != '\0')
        {
            while (UARTFR_0_R & 0x20); // wait until transmit FIFO empty

            UARTDR_0_R = *hello;

            hello++;
        }

        UARTICR_B_R |= 0x10;
}

void init_gps(void)
{
    // Enable UART module 1
    RCGCUART_R |= 0x2;

    // Enable clock for GPIO port B
    RCGCGPIO_R |= 0x2;

    // Enable AHB
    SYSCTL_GPIOHBCTL_R |= 0x2;
    delayByMs(50);

    // Enable digital function for pins 0 and 1 on GPIO port B
    GPIODEN_B_R |= 0x3;

    // Select alternate function for PB0 and PB1
    GPIOAFSEL_B_R |= 0x3;

    // Set pins 0 and 1 to 2mA drive
    GPIODR2R_B_R |= 0x3;

    // Select UART peripheral signal for PB0 and PB1
    GPIOPCTL_B_R &= ~0xFF;
    GPIOPCTL_B_R |= 0x11;

    UARTCTL_B_R &= ~0x1; // disable UART

    // flush FIFO in UARTLCRH
    UARTLCRH_B_R &= ~0x10;

    // Set the baud rate to 115200
    UARTCTL_B_R &= ~0x20; // Set ClkDiv to 16
    UARTIBRD_B_R = 8;
    UARTFBRD_B_R = 44;

    // Set UART clock to sys clock
    UARTCC_B_R = 0x0;

    // Enable the RX interrupt 
    UARTIM_B_R |= 0x10;

    // Trigger RX interrupts when the input buffer is 1/8 full (every 2 bytes). As a side effect TX is also triggered at this fill level, but doesn't matter because we're not enabling that interrupt
    UARTIFLS_B_R &= ~0x3F; 

    // Enable interrupts for UART1 receive in the interrupt controller
    NVIC_EN0_R |= UART1_RXEN;

    // Set word length to 8 bits, and re-enable FIFO
    UARTLCRH_B_R |= 0x70;

    UARTCTL_B_R |= 0x301; // enable UART
}

void init_serial_output(void)
{
    // Enable UART module 0
    RCGCUART_R |= 0x1;
    
    // Enable clock for GPIO port A
    RCGCGPIO_R |= 0x1;

    // Enable AHB
    SYSCTL_GPIOHBCTL_R |= 0x1;
    delayByMs(50);
    
    // Enable digital function for port A pins 0/1
    GPIODEN_A_R |= 0x3;
    
    // Enable alternate function for port A pins 0/1
    GPIOAFSEL_A_R |= 0x3;
    
    // Set port A pins 0/1 to 2 mA drive strength
    GPIODR2R_A_R |= 0x3;
    
    // Assign RX/TX signals to port A pins 0/1
    GPIOPCTL_A_R &= ~0xFF;
    GPIOPCTL_A_R |= 0x11;
    
    // Disable UART for configuration
    UARTCTL_A_R &= ~0x1;
    
    // Flush FIFO
    UARTLCRH_A_R &= ~0x10;
    
    // Set baud rate to 115200
    UARTIBRD_A_R = 8;
    UARTFBRD_A_R = 44;
    UARTCTL_A_R &= ~0x20; // set ClkDiv to 16
    
    // Enable FIFO
    UARTLCRH_A_R |= 0x70;
    
    // Set clock source to system clock
    UARTCC_A_R = 0;
    
    // Re-enable UART for transmission
    UARTCTL_A_R |= 0x301;
}

int main(void)
{
    init_gps();
    init_serial_output();
    // uint8_t gps_byte;
    // bool capturing = false;
    // bool doneCapturing = false;
    // uint8_t NMEA_buffer[128];
    // uint8_t idx = 0;
    // uint8_t* NMEA_ptr;

    // while (true)
    // {
    //     do {
    //         // read until we get the full sentence
    //         while (!(UARTFR_1_R | 0x40)); // Wait until receive FIFO full
            
    //         gps_byte = UARTDR_1_R; // Ignoring error bits for now

    //         if (gps_byte == '$' && !capturing)
    //         {
    //             memset(NMEA_buffer, 0, sizeof(NMEA_buffer));
    //             capturing = true;
    //             NMEA_buffer[idx++] = gps_byte;
    //         }
    //         else if (capturing)
    //         {
    //             if (gps_byte == '\r')
    //             {
    //                 while (!(UARTFR_1_R & 0x40)); // Wait until receive FIFO full
                    
    //                 gps_byte = UARTDR_1_R; // Ignoring error bits for now

    //                 if (gps_byte == '\n')
    //                 {
    //                     capturing = false;
    //                     doneCapturing = true;
    //                     NMEA_buffer[idx++] = '\0';
    //                     idx=0;
    //                 }
    //             }
                
    //             else 
    //             {
    //                 NMEA_buffer[idx++] = gps_byte;
    //             }
    //         }
    //     } while (!doneCapturing);
        
    //     doneCapturing = false;

    //     NMEA_ptr = NMEA_buffer;
        
    //     // drop sentence in byte by byte
    //     while (*NMEA_ptr != '\0')
    //     {
    //         while (UARTFR_0_R & 0x20); // wait until transmit FIFO empty

    //         UARTDR_0_R = *NMEA_ptr;

    //         NMEA_ptr++;
    //     }
    // }
}
