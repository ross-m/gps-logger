# GPS Logger
[GPS logger for the TM4C123GH6PM microcontroller.](https://www.ti.com/lit/ds/symlink/tm4c123gh6pm.pdf)

Baremetal program (no TivaWare libraries) to read GPS data over UART, parse it, and log to console via UART. 

*NOTE:* This project is platform specific in that it relies on an In-Circuit Debug Interface chip for serial output. My code assumes that such a chip exists at UART module 0 and implements a virtual COM port, but if you plan on running it in a different system, 
you'll have to implement USB CDC or use a USB-UART bridge.

## Features
- Interrupt-driven input handling
- State-machine for lightweight, deterministic NMEA sentence buffering
- Designed for extensibility and ISR-safe modularity

### Dependencies

- [Tiva C LaunchPad Microcontroller (EK-TM4C123GXL)](https://www.ti.com/tool/EK-TM4C123GXL#tech-docs)
- [RYS352A GPS module](https://reyax.com/products/RYS352A)
- Code Composer Studio
- [minnmea library (for sentence filtering)](https://github.com/kosma/minmea)

### Build Instructions

1. Clone the repository
2. Install Code Composer Studio with support for the TM4C12x family and TI ARM compiler. Include the TivaWare SDK for tooling.
3. Connect your board over USB, build the project, and flash it to the board. 
4. Open a serial terminal (I used putty) and set the baud rate to 115200. You should see GPS sentences start showing up in your terminal!

### Reflection
The most difficult part of this project was balancing all of the needs of the system.

My main requirement was that I use an ISR to handle input rather than a busy loop, as this blocks CPU and doesn't scale well if you want to integrate other peripherals. This was never in the plan for my project, but I wanted to learn the idiomatic / industry accepted way of doing things.

Out of this requirement arose a second: if handling data byte-by-byte (this MCU offers a byte-width FIFO for UART applications), I must keep the ISR lightweight. If too much time is spent handling one interrupt, that invocation could be interrupted by the next byte arriving in the queue. This could lead to lost or corrupted data.

The third requirement came from the data interface with the GPS. The RYS352A, like other GPS modules, uses the NMEA standard to send data. This data comes in the form of 'sentences' which begin with a '$' and end with '\r\n'. 

It didn't seem appropriate to have the interrupt routine handle parsing and sending, even if it could do those quickly enough to beat the next byte. From a system design perspective, it felt correct to have the ISR dump each byte onto a queue,
and to have a busy-loop wait until that queue was ready for further processing. That approach makes it easier to add functionality in the future without challenging the underlying assumptions of the handler (that all the work it's doing can be completed within the
sending rate of the GPS). Modularity is important for efficiency and maintainability. 

But then came the central contradiction: how do I define when the buffer is 'ready' if I'm not parsing the sentence? This is something I wrestled with for a while. I considered the following approaches:

Approach 1: In the ISR, timestamp each byte, and monitor for a 'significant' amount of time elapsing since the last byte was received to mark the end of a transmission event.
Why not: I didn't like the idea of relying on some fuzzy notion of expected latency to mark the end of an event. It's nondeterministic: anything that interferes with the transmission of a bit could potentially throw off the entire program.

Approach 2: In the main loop, periodically attempt to parse whatever is in the buffer. 
Why not: There is no concurrency control here. The ISR could potentially interrupt the main loop in the middle of a non-atomic instruction and modify something that is being read, which could break parsing.

Approach 3: Use a state machine to track when a complete sentence has been read, and set a flag telling the main loop that it can go read the data. 

As the code suggests, I decided to go with approach 3. There are still weaknesses to this approach, the main one being it assumes that we can always parse and transmit a full sentence within the update interval of the GPS (1 hZ by default). I mitigated this by disabling interrupts
while processing the buffer. This comes with the possibility of data loss, but I figured that was a better outcome than potentially corrupting memory or logging a malformed sentence. 
