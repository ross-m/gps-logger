#include <systick.h>
#include <registers.h>

static volatile bool buffer_surrendered = false;
static volatile char sentence[128];
static volatile uint8_t idx = 0;

/* 
*  Purpose: Respond to RX interrupts on the UART1 module. State-machine-esque handling to ensure full sentences are parsed.
*  Parameters: void
*  Returns: void
*  Assumes: Sentences are formatted per NMEA 0183 V4.10. Assumes a sentence begins with '$', ends with '\n', and no '\n' is encountered until the end of a sentence.
*/
void UART1_handler(void)
{
    static bool capturing = false;
    static uint8_t next_byte;
    
    while (!(UARTFR_1_R & 0x10)) // Until we have drained the FIFO
    {
        next_byte = UARTDR_1_R;

        if (buffer_surrendered) continue; // If we have read a full sentence and passed flow back to the polling loop, discard everything

        if (capturing) // We have already started reading a sentence
        {
            if (next_byte == '\n') // If the sentence is over, tell the main loop and reset the state of the handler
            {
                sentence[idx++] = '\0';
                capturing = false;
                buffer_surrendered = true;
                break;
            }
            else // Otherwise, ingest this byte
            {
                sentence[idx++] = (char)next_byte;
            }
        }
        else if (next_byte == '$') // We are just beginning to read a sentence
        {
            idx = 0;
            sentence[idx++] = (char)next_byte;
            capturing = true;
        }
    } 

    UARTICR_B_R |= 0x10;
}

/* 
*  Purpose: Configure the GPS module to output GGA sentences
*  Parameters: void
*  Returns: void
*  Assumes: Assumes that the ACK response is sent within 1ms of receiving the last byte. Not super important because we filter by sentence type in the main loop.
*/
void configure_gps_output(void)
{
    char* config[] = // Disable every sentence except GGA
    {
        "$PAIR062,0,1*3F\r\n\0",
        "$PAIR062,1,0*3F\r\n\0",
        "$PAIR062,2,0*3C\r\n\0",
        "$PAIR062,3,0*3D\r\n\0",
        "$PAIR062,4,0*3A\r\n\0",
        "$PAIR062,5,0*3B\r\n\0",
        "$PAIR062,6,0*38\r\n\0",
        "$PAIR062,7,0*39\r\n\0",
        "$PAIR062,8,0*36\r\n\0",
        "$PAIR062,9,0*37\r\n\0"
    };

    char* current_config;
    uint8_t discard;

    for (int i = 0;i < 10;i++) // Send the desired config for each sentence
    {
        current_config = config[i];

        while (*current_config != '\0')
        {
            while (UARTFR_1_R & 0x20); 

            UARTDR_1_R = (uint8_t)*current_config;

            current_config++;
        }

        while (!(UARTFR_1_R & 0x80)); // wait until last byte sent

        delayByMs(1);

        while (!(UARTFR_1_R & 0x10))
        {
            discard = UARTDR_1_R; // Discard the ACK
        }
    }
}

/* 
*  Purpose: Initialize the UART module attached to the GPS
*  Parameters: void
*  Returns: void
*  Assumes: nothing
*/
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

    // // Enable the RX interrupt 
    UARTIM_B_R |= 0x10;

    // Trigger RX interrupts when the input buffer is 1/8 full (every 2 bytes). As a side effect TX is also triggered at this fill level, but doesn't matter because we're not enabling that interrupt
    UARTIFLS_B_R &= ~0x3F; 

    // Enable interrupts for UART1 receive in the interrupt controller
    NVIC_EN0_R |= UART1_RXEN;

    // Set word length to 8 bits, and re-enable FIFO
    UARTLCRH_B_R |= 0x70;

    // enable UART
    UARTCTL_B_R |= 0x301; 
}

/* 
*  Purpose: Initialize the UART module attached to the on-board ICDI (UART0)
*  Parameters: void
*  Returns: void
*  Assumes: nothing
*/
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

/* 
*  Purpose: Enable interrupts on UART1
*  Parameters: void
*  Returns: void
*  Assumes: nothing
*/
void enable_gps_interrupt(void)
{
    UARTIM_B_R |= 0x10;
}

/* 
*  Purpose: Disable interrupts on UART1
*  Parameters: void
*  Returns: void
*  Assumes: nothing
*/
void disable_gps_interrupt(void)
{
    UARTIM_B_R &= ~0x10;
}

/* 
*  Purpose: Initialize the UART ports, then poll for sentences buffered by the interrupt handler
*  Parameters: void
*  Returns: void
*  Assumes: Copying the buffer can take place before the next sentence is sent a second later
*/
int main(void)
{
    init_gps();
    init_serial_output();
    disable_gps_interrupt();
    configure_gps_output();
    enable_gps_interrupt();
    char parse_buffer[128];
    char* stream_ptr = NULL;
    struct minmea_sentence_gga frame;

    while (1)
    {
        while (buffer_surrendered == false); // Wait until the interrupt handler has buffered a full sentence

        disable_gps_interrupt(); // Enter critical section. Risk of data loss, but safer than stomping on shared memory.
        memcpy((void*)parse_buffer, (void*)sentence, idx); // Copy the sentence over to give us a little extra time to parse
        memset((void*)sentence, 0, idx); // Reset the buffer
        buffer_surrendered = false;
        enable_gps_interrupt(); // Exit critical section
       
        if (minmea_parse_gga(&frame, parse_buffer)) // Occassionally we get status messages from the GPS. Ignore those.
        {
            stream_ptr = parse_buffer;

            while (*stream_ptr != '\0')
            {
                while (UARTFR_0_R & 0x20);

                UARTDR_0_R = *stream_ptr;

                stream_ptr++;
            }
        }
    }
}
