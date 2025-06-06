#include <SysTick.h>

/* 
*  Purpose: Block CPU for approximately *ms* milliseconds. Resolution is fuzzy because of the polling: the read is not guaranteed
*            to occur exactly when the wrap does because of the cycles consumed executing the looping instructions. For precise timing
*            needs, use an interrupt.
*  Parameters: ms - the number of milliseconds to wait for. Can't wait more than UINT32_MAX milliseconds because of implementation.
*  Returns: void
*  Assumes: nothing
*/
void delayByMs(uint32_t ms)
{
    if (ms > UINT32_MAX)
    {
        return;
    }

    uint32_t remainingMs = ms;

    // Wait 1 MS at a time
    STRELOAD = CYCLESPMS;

    // clear the cur value register by writing an arbitrary value
    STCURRENT |= 0x1;

    // configure the counter for non-interrupt mode and 80 MHz clock system clock backing
    STCTRL |= 0x5;

    while (remainingMs > 0)
    {
        if ((STCTRL & COUNTMASK) != 0) // works because the count bit clears on read
        {
            remainingMs--;
        }
    }

    // disable the timer
    STCTRL &= ~(0x7);
}
