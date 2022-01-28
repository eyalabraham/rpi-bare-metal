# Bare-metal application libraries

Review the top level [README](../README.md) for a description of this repository and useful references.

## BCM2835 IO library

Static library that implements and interface to various low level IO functions of the BCM2835. The library is implemented as a set of modules each one dedicated to a specific interface. Header ```bcm2835.h``` provides all the necessary definitions to write custom drivers, alternatively use the aproriate ready-made driver modules. The modules are compiled into a static library called ```libgpio```.  

- ```bcm2835.h``` general header with BCM2835 GPIO and peripheral definitions
- ```gpio.c``` discrete GPIO pin input/output functions
- ```auxuart.c``` Auxiliary UART (UART1) driver interface
- ```timer.c``` System Timer driver interface
- ```irq.c``` interrupt management interface ([more comments](../doc/interrupts.md))
- ```spi.c``` SPI0 driver interface
- Video display

## Custom printf() library

All projects use a specialized [printf() implementation by Marco Paland](https://github.com/mpaland/printf). This implementation requires linking a custom character output function ```_putchar()``` as well as ```libgcc```'s ```__aeabi_idivmod``` functions for modulus calculation. The ```_putchar()``` function should be implemented in the application using ```printf()```, and the division/modulus function are linked through ```libgcc```. A header file ```printf_config.h``` is defined and used to configure the module's features.  

> In order to successfully link ```libgcc``` when using the **ld** linker tool, the ```-lprintf``` must appear before the ```-lgcc``` on the linker command line. The order is important given the way the linker processes symbols.

> The BCM2835 Auxiliary UART (UART1) must be initialized in the application for the ```printf()``` function to work.

## RPi mailbox interface library

Source by Brian Sidebotham from [GitHub sample](https://github.com/BrianSidebotham/arm-tutorial-rpi/tree/master/part-5/armc-016)

## Notes about BCM2835

I found that the information in the peripherals' manual may not be 100% correct and sometimes missing crucial information on how to work with the registers. It is highly advisable to follow these few guidelines:  

- Follow the [BCM2835 datasheet errata](https://elinux.org/BCM2835_datasheet_errata)
- Review code examples in this repository as well as others cited here under resources. Important details such as writing a '1' to a register's bit instead of doing a read/modify/write will make a difference.

## Resources

- [BCM2835ARM Peripherals](https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf)
- [BCM2835 datasheet errata](https://elinux.org/BCM2835_datasheet_errata)

