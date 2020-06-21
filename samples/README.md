# Bare-metal application samples

Review the top level [README](../README.md) for a description of this repository and useful references.  
A set of C and assembly code files and implementation samples. I created these to serve as a starting point for projects or as a demonstration of concepts. The concepts and exceptions listed here will apply throughout.  

> The code in this repository targets Raspberry Pi model B, and Pi Zero or ZeroW.

Code samples:  
- ```led0.c``` classic RPi activity LED blinker and check NewLib linking
- ```led1.c``` RPi activity LED blinker with the GPIO library
- ```uart1.c``` receive and echo serial console with the UART1 GPIO library done with polling
- ```stopwatch1.c``` stopwatch on serial console with the UART1 and System Timer GPIO libraries
- ```uart2.c``` receive and echo serial console character with the UART1 GPIO library running with receive interrupts
- ```stopwatch2.c``` stopwatch on serial console with the UART1 and System Timer GPIO libraries using interrupts from both System Timer and UART1.
- ```a2d.c``` read MAX186 A-to-D converter
- ```fb.c``` graphics on the video frame buffer using mailbox interface, demo with Mandelbrot set fractal. This sample uses the 8-bit per pixel color depth capability documented in [this how-to](../doc/8-bpp.md).

## Raspberry Pi (Model B Rev 2.0) and Zero/Zero-W memory map

### General Raspberry Pi memory map

From the [BCM2835 ARM Peripherals](https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf)

```
      ARM Physical
        Address

    +--------------+ 0x3FFFFFFF
    |              |
    |              |
    |              |
    |              |
    |              |
    +--------------+
    |              |
    | IO           |
    | Peripherals  |
    |              | 0x20000000
    +--------------+
    |              | 0x1FFFFFFF
    |              |
    |              |   (GPU 64MB)
    |              |
    |              | 0x1C000000
    | -- SDRAM --  | 0x1BFFFFFF
    |              |
    |              |
    |              |   (ARM 448MB)
    |              |
    |              |
    |              |
    +--------------+ 0x00000000

```

### Bare-metal application memory map

```
     ARM application
       memory map

    +--------------+ 0x1BFFFFFF
    |              |
    |              |
    |              |
    |     /\       |    User code and data
    |              |
    +--------------+ 0x00008000
    |              |
    |     \/       |    Stack space
    |              |
    |              |
    +--------------+
    |              | 0x0000001F
    |              |
    |              |    Exception vectors
    |              |
    +--------------+ 0x00000000

```
