/*
 * uart2.c
 *
 *  Test the use the Auxiliary UART library with an interrupt
 *  driven character/byte receiver.
 *  Requires a serial terminal set to 57600,N,1
 *
 */

#include    <stdint.h>

#include    "bcm2835.h"
#include    "auxuart.h"
#include    "printf.h"
#include    "irq.h"


/* -----------------------------------------
   Local definitions
----------------------------------------- */
#define     ESC         27

/* -----------------------------------------
   Globals
----------------------------------------- */
uint8_t     text_string[256];

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
    int     byte_count, i;

    /* Initialize the IO system
     */
    disable();

        irq_init();
        bcm2835_auxuart_init(BAUD_57600, 0, 0, AUXUART_ENA_RX_IRQ);

    enable();

    printf("%c[2J", ESC);
    printf("RPi bare-metal %s %s\n", __DATE__, __TIME__ );
    printf("CPU ID: 0x%x\n", machid);
    printf("bcm2835 Auxiliary UART (UART1) receive with IRQ interrupt\n\n");

    /* Character echo loop
     */
    while ( 1 )
    {
        byte_count = bcm2835_auxuart_rx_data(text_string, 256);
        for ( i = 0; i < byte_count; i++ )
        {
            if ( text_string[i] == '\r')
                bcm2835_auxuart_putchr('\n');
            bcm2835_auxuart_putchr(text_string[i]);
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
