/*
 * stopwatch1.c
 *
 *  Test the use the System Timer and Auxiliary UART libraries.
 *  This sample program displays a stop watch on the serial console
 *  and responds to 'S'tart/'S'top/'R'eset keyboard commands.
 *  Stop watch is not accurate due to serial output lag, but the
 *  concept works.
 *
 *  Requires a serial terminal set to 115200,N,1
 *
 */

#include    <stdint.h>

#include    "bcm2835.h"
#include    "auxuart.h"
#include    "timer.h"

#include    "printf.h"

/* -----------------------------------------
   Local definitions
----------------------------------------- */
#define     ESC         27
#define     SW_STOP     0
#define     SW_RUN      1
#define     HUNDRETH    10000

/* -----------------------------------------
   Module globals
----------------------------------------- */
int         hours, minutes, seconds, hundreth_seconds;
uint32_t    time_mark;

int         state = SW_STOP;

/*------------------------------------------------
 * kernel()
 *
 *  C code module entry point.
 *
 *  param:  ATAGs (only machine ID is valid)
 *  return: Nothing
 */
void kernel(uint32_t r0, uint32_t machid, uint32_t atags)
{
    uint8_t byte = 0;

    /* Initialize the IO system
     */
    bcm2835_auxuart_init(BAUD_115200, 0, 0, AUXUART_DEFAULT);

    printf("%c[2J", ESC);
    printf("RPi bare-metal %s %s\n", __DATE__, __TIME__ );
    printf("CPU ID: 0x%x\n", machid);
    printf("bcm2835 System Timer and Auxiliary UART (UART1) stop-watch\n");
    printf("'s' to start/stop, 'r' to reset\n\n");

    /* Stop watch loop
     */
    printf("\n");
    while ( 1 )
    {
        printf("%02d:%02d:%02d.%02d\r", hours, minutes, seconds, hundreth_seconds);

        if ( bcm2835_auxuart_ischar() )
            byte = bcm2835_auxuart_getchr();
        else
            byte = 0;

        if ( byte == 'r')
        {
            hours = 0;
            minutes = 0;
            seconds = 0;
            hundreth_seconds = 0;
            time_mark = bcm2835_st_read();
        }
        else if ( byte == 's' )
        {
            if ( state == SW_STOP )
            {
                state = SW_RUN;
                time_mark = bcm2835_st_read();
            }
            else
            {
                state = SW_STOP;
            }
        }

        if ( state == SW_RUN )
        {
            if ( (bcm2835_st_read() - time_mark) >= HUNDRETH )
            {
                time_mark = bcm2835_st_read();
                hundreth_seconds++;
                if ( hundreth_seconds == 100 )
                {
                    hundreth_seconds = 0;
                    seconds++;
                    if ( seconds == 60 )
                    {
                        seconds = 0;
                        minutes++;
                        if ( minutes == 60 )
                        {
                            minutes = 0;
                            hours++;
                            if ( hours == 25 )
                            {
                                hours = 0;
                            } // Hours
                        } // minutes
                    } // Seconds
                } // Hundredth seconds
            }
        }
    }
}

/*------------------------------------------------
 * _putchar()
 *
 *  Low level character output/stream for printf()
 *
 *  param:  character
 *  return: none
 */
void _putchar(char character)
{
    if ( character == '\n')
        bcm2835_auxuart_putchr('\r');
    bcm2835_auxuart_putchr(character);
}
