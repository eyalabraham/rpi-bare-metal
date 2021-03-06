/*
 * auxspi.h
 *
 *  Header file for the SPI0 interface of the BCM2835.
 *  The module only supports SPI0, SPI1 is available on RPi Zero 40-pin
 *  header and SPI2 is not available on the RPi header pins.
 *  The module does not provide an option to configure interrupts.
 *
 *  This device is configured by default for:
 *  - Interface in SPI mode (not TODO LoSSI mode)
 *  - CS0 selected for automatic assertion
 *  - Clock at Mode-0
 *  - Chip select lines are active low and not automatically un-asserted
 *  - No DMA
 *  - No Tx IRQ
 *  - No Rx IRQ
 *  - Single byte data length
 *  - Data rate 488.28125kHz
 *  - MSB first nit order
 *
 *   Resources:
 *      https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf
 *
 */

#ifndef __SPI_H__
#define __SPI_H__

#include    <stdint.h>

#include    "bcm2835.h"

/* SPI setup options
 * 'OR' these options to configure SPI0 for non-default operation
 */
#define     SPI_DEFAULT                 0x00000000              // See in file title
#define     SPI_CPHA_BEGIN              0x00000001              // First SCLK transition at beginning of data bit
#define     SPI_CPOL_HI                 0x00000002              // Rest state of clock = high
#define     SPI_CSPOL_HI                0x00000004              // Chip select lines are active high
#define     SPI_ENA_DMA                 0x00000008              // Enable DMA
#define     SPI_ENA_TX_IRQ              0x00000010              // Enable transmission done interrupt
#define     SPI_ENA_RX_IRQ              0x00000020              // Enable receive data interrupt
#define     SPI_LONG_DATA               0x00000040              // Set to 32-bit data IO
#define     SPI_LOSSI_MODE              0x00000080              // Interface will be in SoSSI mode

/* SPI clock phase and polarity
 */
typedef enum
{
    SPI0_MODE0 = 0,                     // CPOL = 0, CPHA = 0
    SPI0_MODE1 = 1,                     // CPOL = 0, CPHA = 1
    SPI0_MODE2 = 2,                     // CPOL = 1, CPHA = 0
    SPI0_MODE3 = 3,                     // CPOL = 1, CPHA = 1
} spi_mode_t;

/* Chip select pin(s)
 * https://en.wikipedia.org/wiki/Serial_Peripheral_Interface#/media/File:SPI_timing_diagram2.svg
 */
typedef enum
{
    SPI0_CS0 = 0,                       // Chip Select 0
    SPI0_CS1 = 1,                       // Chip Select 1
    SPI0_CS2 = 2,                       // Chip Select 2 (XXX pins CS1 and CS2 are asserted)
    SPI0_CS_NONE = 3,                   // No automatic CS, application control
} spi_chip_sel_t;

/* Resulting data rates are based on 250MHz core clock speed.
 * Using 'power of 2' dividers, although errata indicates otherwise.
 */
typedef enum
{
    SPI0_DATA_RATE_4KHZ   = 0,          //   3.814697260kHz
    SPI0_DATA_RATE_8KHZ   = 32768,      //   7.629394531kHz
    SPI0_DATA_RATE_15KHZ  = 16384,      //  15.25878906kHz
    SPI0_DATA_RATE_30KHZ  = 8192,       //  30.51757813kHz
    SPI0_DATA_RATE_61KHZ  = 4096,       //  61.03515625kHz
    SPI0_DATA_RATE_122KHZ = 2048,       // 122.0703125kHz
    SPI0_DATA_RATE_244KHZ = 1024,       // 244.140625kHz
    SPI0_DATA_RATE_488KHZ = 512,        // 488.28125kHz
    SPI0_DATA_RATE_976KHZ = 256,        // 976.5625kHz
    SPI0_DATA_RATE_2MHZ   = 128,        //   1.953125MHz
    SPI0_DATA_RATE_4MHZ   = 64,         //   3.90625MHz
    SPI0_DATA_RATE_8MHZ   = 32,         //   7.8125MHz
    SPI0_DATA_RATE_15MHZ  = 16,         //  15.625MHz
    SPI0_DATA_RATE_31MHZ  = 8,          //  31.25MHz
    SPI0_DATA_RATE_62MHZ  = 4,          //  62.5MHz
    SPI0_DATA_RATE_125MHZ = 2,          // 125.0MHz
} spi_clock_div_t;


int  bcm2835_spi_init(uint32_t configuration);              // Initialization
void bcm2835_spi_close(void);                               // Close SPI0 device
void bcm2835_spi_set_rate(spi_clock_div_t data_rate);       // Set SPI transfer data rate
void bcm2835_spi_clk_mode(spi_mode_t mode);                 // Set SPI clock mode (CPOL/CPHA)
void bcm2835_spi_cs(spi_chip_sel_t cs);                     // Select CS line
void bcm2835_spi_cs_polarity(spi_chip_sel_t cs, int level); // Set CS polarity

void bcm2835_spi_transfer_Ex(uint8_t *tx_buf, uint8_t *rx_buf, uint32_t count);
void bcm2835_spi_send_byte(uint8_t byte);                   // Transmit one byte
int  bcm2835_spi_recv_byte(void);                           // Receive one byte
int  bcm2835_spi_transfer_byte(uint8_t tx_byte);            // Transmit a byte and return the received byte

#endif  /* __SPI_H__ */
