/*
 * a2d.c
 *
 *  Test the use the SPI0 with a MAX186 A-to-D converter.
 *  Requires a serial terminal set to 9600,N,1
 *
 */

#include    <stdint.h>
#include    <string.h>

#include    "printf.h"
#include    "bcm2835.h"
#include    "auxuart.h"
#include    "spi0.h"
#include    "timer.h"

/* -----------------------------------------
   Local definitions
----------------------------------------- */
#define     ESC             27
#define     AD_CONTROL      0x8f    // Uni-polar, single-ended, external-clock conversion

/* -----------------------------------------
   Module static functions
----------------------------------------- */
static int a2d_read(int channel);

/* -----------------------------------------
   Module globals
----------------------------------------- */
uint8_t channel_id[] = {0x00, 0x40, 0x10, 0x50, 0x20, 0x60, 0x30, 0x70};

uint8_t tx_buf[4];
uint8_t rx_buf[4];

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
    int     channel = 0;

    /* Configure IO
     */
    bcm2835_auxuart_init(BAUD_9600, 0, 0, AUXUART_DEFAULT);

    bcm2835_spi0_init(SPI0_DEFAULT);
    bcm2835_spi0_set_rate(SPI0_DATA_RATE_61KHZ);

    /* Print some text
     */
    printf("%c[2J", ESC);
    printf("RPi bare-metal %s %s\n", __DATE__, __TIME__ );
    printf("CPU ID: 0x%x\n", machid);
    printf("bcm2835 SPI0 reading MAX186 serial A-to-D\n\n");

    /* A-to-D read and display value loop
     */
    while ( 1 )
    {
        printf("Channel %1d readout %5d\r", channel, a2d_read(channel));
    }
}

/* ----------------------------------------------------------------------------
 * a2d_read()
 *
 * Read an A-to-D channel.
 * This function blocks until a conversion is ready, and then returns the 12-bit
 * A-to-D converter value.
 *  * Function assumes that IO ports are configured!
 *
 * param:   A-to-D channel to read, one of eight channels 0 to 7
 * return:  12-bit A-to-D conversion
 *
 */
int a2d_read(int channel)
{
    int     conversion;

    if ( channel > 7 )
        return 0;

    /* Send control byte to select channel and
     * start the conversion
     */
    memset(tx_buf, 0, sizeof(tx_buf));
    memset(rx_buf, 0, sizeof(rx_buf));

    tx_buf[0] = channel_id[channel] | AD_CONTROL;
    bcm2835_spi0_transfer_Ex(tx_buf, rx_buf, 3);

    conversion = (int)(((rx_buf[1] << 8) + rx_buf[2]) >> 4);

    return conversion;
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
