/*
 * led1.c
 *
 *  Main code module to blink the Activity LED using the GPIO library.
 *
 *   Resources:
 *      https://github.com/BrianSidebotham/arm-tutorial-rpi by Brian Sidebotham
 *
 */

#include    <stdint.h>
#include    <string.h>

#include    "bcm2835.h"
#include    "gpio.h"

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
    /*   Function code goes here, and would be replaced with you code ---->
     */

        /* Enable GPIO16 as an output
         */
        bcm2835_gpio_fsel(RPIB_ACT_LED, BCM2835_GPIO_FSEL_OUTP);

        /* Never exit as there is no OS to exit to!
         */
        while (1)
        {
            bcm2835_crude_delay(500000);

            /* Set the GPIO pin high
             */
            bcm2835_gpio_set(RPIB_ACT_LED);

            bcm2835_crude_delay(500000);

            /* Set the GPIO pin low
             */
            bcm2835_gpio_clr(RPIB_ACT_LED);
        }

    /*   <---- End of function code
     */
}
