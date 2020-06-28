/*
 * uart1.c
 *
 *  Test the use the Auxiliary UART library.
 *  Print out some test, CPU ID, and run a keyboard echo loop.
 *  Requires a serial terminal set to 57600,N,1
 *
 */

#include    <stdint.h>

#include    "bcm2835.h"
#include    "auxuart.h"

char    message[] = {"\r\nRPi bare-metal\r\nbcm2835 GPIO and Auxiliary UART (UART1)\r\nCPU ID: 0x\0"};

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
    int     j;
    uint8_t byte;
    char   *i;

    /* Initialize the IO system
     */
    bcm2835_auxuart_init(BAUD_57600, 0, 0, AUXUART_DEFAULT);

    /* Message
     */
    for ( i = message; *i; i++ )
    {
        bcm2835_auxuart_putchr(*i);
    }

    /* CPU ID:
     *  PI3 0x410FD034
     *  PI2 0x410FC075
     *  PI1 0x410FB767
     */
    for ( j = 0; j < 8; j++ )
    {
        byte = (uint8_t)((machid & 0xf0000000) >> 28);
        if ( byte < 10 )
            byte += 48; // ASCII '0'..'9'
        else
            byte += 87; // ASCII 'a'..'f'
        bcm2835_auxuart_putchr(byte);
        machid = machid << 4;
    }

    bcm2835_auxuart_putchr('\r');
    bcm2835_auxuart_putchr('\n');

    /* Character echo loop
     */
    while ( 1 )
    {
        byte = bcm2835_auxuart_waitchr(); //  Get character,
        if ( byte == '\r')
            bcm2835_auxuart_putchr('\n');
        bcm2835_auxuart_putchr(byte);     //  and echo it back.
    }
}
