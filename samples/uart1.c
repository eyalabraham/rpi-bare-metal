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
#include    "mailbox.h"
#include    "printf.h"

/* -----------------------------------------
   Module static functions
----------------------------------------- */
void halt(char *msg);

/* -----------------------------------------
   Module globals
----------------------------------------- */
char    message[] = {"\nRPi bare-metal\nbcm2835 GPIO and Auxiliary UART (UART1)\nCPU ID: \0"};

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
    uint8_t     byte;
    uint32_t    system_clock;
    uint32_t    baud_rate_div;

    mailbox_tag_property_t *mp;

    /* Initialize the IO system
     */
    bcm2835_auxuart_init(BAUD_57600, 0, 0, AUXUART_DEFAULT);

    /* Message
     */
    printf("%s", message);

    /* CPU ID:
     *  PI3   0x410FD034
     *  PI2   0x410FC075
     *  PI1/0 0x410FB767
     */
    printf("0x%08x\n", machid);

    /* Retrieve core clock speed
     */
    bcm2835_mailbox_init();
    bcm2835_mailbox_add_tag(TAG_GET_CLOCK_RATE, TAG_CLOCK_CORE);
    //bcm2835_mailbox_add_tag(TAG_GET_CLOCK_RATE, TAG_CLOCK_ARM);
    //bcm2835_mailbox_add_tag(TAG_GET_CLOCK_RATE, TAG_CLOCK_UART);
    //bcm2835_mailbox_add_tag(TAG_GET_CLOCK_RATE, TAG_CLOCK_ISP);

    if ( !bcm2835_mailbox_process() )
        halt("Mailbox call failed.");

    mp = bcm2835_mailbox_get_property(TAG_GET_CLOCK_RATE);
    if ( mp )
    {
        system_clock = mp->values.fb_alloc.param2;
        baud_rate_div = (system_clock / (57600 * 8)) - 1;
        printf("Core clock: %u[Hz]\n", system_clock);
        printf("  Aux UART baud rate divisor: %u\n", baud_rate_div);
    }
    else
        halt("failed TAG_CLOCK_CORE");

    /* Retrieve ARM clock speed
     */
    bcm2835_mailbox_init();
    //bcm2835_mailbox_add_tag(TAG_GET_CLOCK_RATE, TAG_CLOCK_CORE);
    bcm2835_mailbox_add_tag(TAG_GET_CLOCK_RATE, TAG_CLOCK_ARM);
    //bcm2835_mailbox_add_tag(TAG_GET_CLOCK_RATE, TAG_CLOCK_UART);
    //bcm2835_mailbox_add_tag(TAG_GET_CLOCK_RATE, TAG_CLOCK_ISP);

    if ( !bcm2835_mailbox_process() )
        halt("Mailbox call failed.");

    mp = bcm2835_mailbox_get_property(TAG_GET_CLOCK_RATE);
    if ( mp )
    {
        printf("ARM clock: %u[Hz]\n", mp->values.fb_alloc.param2);
    }
    else
        halt("failed TAG_CLOCK_CORE");

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

/*------------------------------------------------
 * halt()
 *
 *  Print message and halt in endless loop
 *
 *  param:  Message
 *  return: none
 */
void halt(char *msg)
{
    printf("%s\nHalted.\n", msg);

    while (1)
    {
        /* Do nothing */
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
