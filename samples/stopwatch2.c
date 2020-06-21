/*
 * stopwatch2.c
 *
 *  Test the use the System Timer and Auxiliary UART libraries with interrupts.
 *  This sample program displays a stop watch on the serial console
 *  and responds to 'S'tart/'S'top/'R'eset keyboard commands.
 *
 *  Requires a serial terminal set to 9600,N,1
 *
 */

#include    <stdint.h>

#include    "printf.h"
#include    "bcm2835.h"
#include    "auxuart.h"
#include    "irq.h"
#include    "timer.h"

/* -----------------------------------------
   Local definitions
----------------------------------------- */
#define     ESC         27
#define     SW_STOP     0
#define     SW_RUN      1
#define     HUNDRETH    10000

/* -----------------------------------------
   Module static functions
----------------------------------------- */
static void system_timer_isr(void);

/* -----------------------------------------
   Module globals
----------------------------------------- */
volatile int hours, minutes, seconds, hundreth_seconds;
volatile int state = SW_STOP;

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

    disable();

        /* Initialize system timer with interrupts.
         * The order makes this higher priority than UART1.
         */
        irq_init();
        irq_register_handler(IRQ_SYSTEM_TIMER1, system_timer_isr);
        bcm2835_st_set_compare(ST_COMPARE1, HUNDRETH);
        bcm2835_st_clr_compare_match(ST_COMPARE1);
        irq_enable(IRQ_SYSTEM_TIMER1);

        /* Initialize UART1
         */
        bcm2835_auxuart_init(BAUD_9600, 0, 0, AUXUART_ENA_RX_IRQ);

    enable();

    /* Print some text
     */
    printf("%c[2J", ESC);
    printf("RPi bare-metal %s %s\n", __DATE__, __TIME__ );
    printf("CPU ID: 0x%x\n", machid);
    printf("bcm2835 System Timer and Auxiliary UART (UART1) stop-watch\n");
    printf("'s' to start/stop, 'r' to reset\n\n");

    /* Stop watch loop
     */
    while ( 1 )
    {
        printf("%02d:%02d:%02d.%02d\r", hours, minutes, seconds, hundreth_seconds);

        if ( !bcm2835_auxuart_rx_byte(&byte) )
            continue;

/*
        if ( bcm2835_auxuart_ischar() )
            byte = bcm2835_auxuart_getchr();
        else
            byte = 0;
*/

        if ( byte == 'r')
        {
            disable();

            hours = 0;
            minutes = 0;
            seconds = 0;
            hundreth_seconds = 0;
            bcm2835_st_set_compare(ST_COMPARE1, HUNDRETH);

            enable();
        }
        else if ( byte == 's' )
        {
            if ( state == SW_STOP )
            {
                state = SW_RUN;
                bcm2835_st_clr_compare_match(ST_COMPARE1);
                bcm2835_st_set_compare(ST_COMPARE1, HUNDRETH);
                irq_enable(IRQ_SYSTEM_TIMER1);
            }
            else
            {
                state = SW_STOP;
                bcm2835_st_clr_compare_match(ST_COMPARE1);
                irq_disable(IRQ_SYSTEM_TIMER1);
            }
        }
    }
}

/*------------------------------------------------
 * system_timer_isr()
 *
 *  This ISR will be triggered by a system timer interrupts
 *  every 1/100 of a second, and will advance the stop watch counters.
 *
 *  param:  none
 *  return: none
 */
void system_timer_isr(void)
{
    if ( state == SW_RUN )
    {
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

    bcm2835_st_clr_compare_match(ST_COMPARE1);
    bcm2835_st_set_compare(ST_COMPARE1, HUNDRETH);
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
