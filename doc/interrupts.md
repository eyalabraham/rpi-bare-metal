
# Interrupt module

Notes captured around interrupt handling. Also reference chapter 7 on the BCM2835 peripherals document.  

Notes:
- All interrupts generated by the arm control block are level sensitive
- All interrupts remain asserted until disabled or the interrupt source is cleared
- The interrupts from doorbell 0,1 and mailbox 0 go to the ARM
- No interrupt vectoring, interrupts are signaled through:
    - Three interrupt pending registers.
    - One basic pending register and two GPU pending registers.
    - The basic pending register can be checked to poll if the two GPU pending registers have a pending interrupt.
- All interrupts are routed to the ARM IRQ, multiple IRQs can be used
    - The IRQ interrupt service must check which one was generated
    - The IRQ interrupt service could implement some priority scheme
- Only **one** FIQ can be routed to the ARM
    - FIQ selection through register 'FIQ control' at offset 0x20C
- Interrupts need to be enabled in all three layers
    - Device/peripheral generating the interrupt
    - Interrupt controller
    - ARM IRQ and/or FIQ bits

## Implementation

- For simplicity, only handle IRQ requests. There are not FIQ request that are not covered by and IRQ
- IRQ service routine is a dispatcher:
    - Scans the enabled interrupts for pending requests.
    - Calls the registered device service handler if one exists.
    - Can implement a priority scheme.

## Definitions

```
/* Interrupt controller registers
 */
#define     INT_BASE                    0x2000B000

#define     INT_IRQ_BASIC_PEND          0x200
#define     INT_IRQ_PEND1               0x204
#define     INT_IRQ_PEND2               0x208
#define     INT_FIQ_CTRL                0x20C
#define     INT_IRQ_ENA1                0x210
#define     INT_IRQ_ENA2                0x214
#define     INT_IRQ_BASIC_ENA           0x218
#define     INT_IRQ_DIS1                0x21C
#define     INT_IRQ_DIS2                0x220
#define     INT_IRQ_BASIC_DIS           0x224

/* Interrupt masking bits for basic IRQ registers
 */
#define     INT_MASK_PEND2              0x00000200      // Pending request in IRQ2 pending
#define     INT_MASK_PEND1              0x00000100      // Pending request in IRQ1 pending
#define     INT_MASK_DBELL1             0x00000008      // ARM Doorbell 1 IRQ pending
#define     INT_MASK_DBELL0             0x00000004      // ARM Doorbell 0 IRQ pending
#define     INT_MASK_MAILBOX            0x00000002      // ARM Mailbox IRQ pending
#define     INT_MASK_TIMER              0x00000001      // ARM Timer IRQ pending

/* Interrupt masking bits for IRQ-1 registers
 */
#define     INT_MASK_SYS_TIMER0         0x00000001      // System Timer Compare 0 (already used by the GPU.)
#define     INT_MASK_STS_TIMER1         0x00000002      // System Timer Compare 1
#define     INT_MASK_SYS_TIMER2         0x00000004      // System Timer Compare 2 (already used by the GPU.)
#define     INT_MASK_SYS_TIMER3         0x00000008      // System Timer Compare 3
#define     INT_MASK_USB_CTRL           0x00000200      // USB Controller
#define     INT_MASK_AUX                0x20000000      // Auxiliary comm (UART1, SPI1, SPI2)

/* Interrupt masking bits for IRQ-2 registers
 */
#define     INT_MASK_I2C1               0x00000800      // I2C1
#define     INT_MASK_SPISL              0x00000800      // SPI slave
#define     INT_MASK_PWA0               0x00002000      // PWA0
#define     INT_MASK_PWA1               0x00004000      // PWA1
#define     INT_MASK_SMI                0x00010000      // Secondary Memory Interface (not implemented)
#define     INT_MASK_GPIO0              0x00020000      // GPIO0 (GPIO0..32)
#define     INT_MASK_GPIO1              0x00040000      // GPIO1
#define     INT_MASK_GPIO2              0x00080000      // GPIO2
#define     INT_MASK_GPIO3              0x00100000      // GPIO3
#define     INT_MASK_I2C0               0x00200000      // I2C0
#define     INT_MASK_SPI0               0x00400000      // SPI0
#define     INT_MASK_PCM                0x00800000      // PCM
#define     INT_MASK_UART0              0x02000000      // UART0

/* FIQ selection
 */
#define     INT_FIQ_ENA                 0x00000080      // Enable FIQ selection
#define     INT_FIQ_MASK                0xFFFFFF80      // Mask FIQ selection
#define     INT_FIQ_ARM_TIMER           64              // ARM Timer interrupt
#define     INT_FIQ_MAILBOX             65              // ARM Mailbox interrupt
#define     INT_FIQ_DBELL0              66              // ARM Doorbell 0 interrupt
#define     INT_FIQ_DBELL1              67              // ARM Doorbell 1 interrupt

typedef enum
{
} int_source_t;
```

Interesting comments:  

```
/* *** https://elixir.bootlin.com/linux/v4.6/source/drivers/irqchip/irq-bcm2835.c ***
 *
 * Copyright 2010 Broadcom
 * Copyright 2012 Simon Arlott, Chris Boot, Stephen Warren
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Quirk 1: Shortcut interrupts don't set the bank 1/2 register pending bits
 *
 * If an interrupt fires on bank 1 that isn't in the shortcuts list, bit 8
 * on bank 0 is set to signify that an interrupt in bank 1 has fired, and
 * to look in the bank 1 status register for more information.
 *
 * If an interrupt fires on bank 1 that _is_ in the shortcuts list, its
 * shortcut bit in bank 0 is set as well as its interrupt bit in the bank 1
 * status register, but bank 0 bit 8 is _not_ set.
 *
 * Quirk 2: You can't mask the register 1/2 pending interrupts
 *
 * In a proper cascaded interrupt controller, the interrupt lines with
 * cascaded interrupt controllers on them are just normal interrupt lines.
 * You can mask the interrupts and get on with things. With this controller
 * you can't do that.
 *
 * Quirk 3: The shortcut interrupts can't be (un)masked in bank 0
 *
 * Those interrupts that have shortcuts can only be masked/unmasked in
 * their respective banks' enable/disable registers. Doing so in the bank 0
 * enable/disable registers has no effect.
 *
 * The FIQ control register:
 *  Bits 0-6: IRQ (index in order of interrupts from banks 1, 2, then 0)
 *  Bit    7: Enable FIQ generation
 *  Bits  8+: Unused
 *
 * An interrupt must be disabled before configuring it for FIQ generation
 * otherwise both handlers will fire at the same time!
 */

```

## Function interface

- Initialization (hook dispatcher service routine?)
- Close/shutdown interrupt servicing (needed?)
- Register an interrupt service for a source (source name, service routine address)
- Enable / disable specific source (source name)
- Global IRQ enable disable

## Resources

- BCM2835 peripherals.
- [Embedded Xinu](https://embedded-xinu.readthedocs.io/en/latest/arm/rpi/BCM2835-Interrupt-Controller.html)
- [Xinu](https://github.com/xinu-os/xinu)
- [BCM2835 GPIO Event Detect with interrupt](https://www.raspberrypi.org/forums/viewtopic.php?t=248813)
- [UART with interrupts](https://github.com/dwelch67/raspberrypi/tree/master/uart04)
