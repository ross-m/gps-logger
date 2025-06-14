#ifndef sys_defs
#define sys_defs

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "minmea.h"

void disable_gps_interrupt(void);
void enable_gps_interrupt(void);
void UART1_handler(void);
void configure_gps_output(void);
void init_gps(void);
void init_serial_output(void);

#endif


