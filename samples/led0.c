/*
 * led0.c
 *
 *  Main code module to blink the Activity LED.
 *
 *   Resources:
 *      https://github.com/BrianSidebotham/arm-tutorial-rpi by Brian Sidebotham
 *
 */

#include    <stdint.h>
#include    <string.h>

#include    "rpi-gpio.h"

/* -----------------------------------------
   Globals
----------------------------------------- */
volatile uint32_t  *gpio = (uint32_t*)GPIO_BASE;
volatile uint32_t   tim;

/*------------------------------------------------
 * kernel()
 *
 *  Make code module entry point.
 *
 *  param:  ATAGs (only machine ID is valid)
 *  return: Nothing
 */
void kernel(uint32_t r0, uint32_t machid, uint32_t atags)
{
    /*   Function code goes here, and would be replaced with you code ---->
     */

        /* Using a libc call with Newlib (libc_nano static library)
         */
        memset((void *)gpio, 0, sizeof(gpio));

        /* Write 1 to the GPIO16 init nibble in the Function Select 1 GPIO
           peripheral register to enable GPIO16 as an output
         */
        gpio[LED_GPFSEL] |= (1 << LED_GPFBIT);

        /* Never exit as there is no OS to exit to!
         */
        while (1)
        {
            for (tim = 0; tim < 1000000; tim++)
                ;

            /* Set the LED GPIO pin low (turn OK LED on for original Pi,
               and off for plus models )*/
            gpio[LED_GPCLR] = (1 << LED_GPIO_BIT);

            for (tim = 0; tim < 1000000; tim++)
                ;

            /* Set the LED GPIO pin high ( Turn OK LED off for original Pi, and on
               for plus models )*/
            gpio[LED_GPSET] = (1 << LED_GPIO_BIT);
        }

    /*   <---- End of function code
     */
}
