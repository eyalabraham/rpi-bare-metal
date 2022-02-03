# Raspberry Pi clock

Raspberry Pi clock speeds can be controlled at run time via programed mailbox calls, or statically with definitions in the ```config.txt``` file. Such an example include the following text.

```
# ---- CPU ----
# Clock frequency override to make sure we run at 1000MHz
force_turbo=1
# Settings for Pi0
[pi0]
# default frequencies are 1000 / 400 / 450
arm_freq=1000
core_freq=400
sdram_freq=450
```

In either case, changes in clock speeds influence the clock references that various peripherals use to derive their operating clock. For example the Auxiliary UART Baur rate or the SPI controller data clock rate.

## Peripheral clock sources

From the BCM2835 ARM Peripherals manual

| Devic     | Page | Register      | Address offset | Clock source         | Default rate | Notes                   |
|-----------|------|---------------|----------------|----------------------|--------------|-------------------------|
| Aux UART  |  11  | AUX-MU-BAUD   | 0x7E21 5068    | core_clock           | 250MHz       |                         |
| AUX SPI1  |  21  | AUXSPI0-CNTL0 | 0x7E21 5080    | core_clock           | 250MHz       |                         |
| AUX SPI2  |  21  | AUXSPI1-CNTL0 | 0x7E21 50C0    | core_clock           | 250MHz       | No user GPIO pin access |
| BSC (I2C) |  34  | CDIV          |                | core_clock           | 250MHz       | Manual states 150Mhz    |
| PCM / I2S | 119  |               |                |                      | ?            |                         |
| PWM       | 138  |               |                |                      | ?            |                         |
| SPI0      | 148  | CDIV          | 0x7E204008     | core_clock           | 250MHz       |                         |
| Timer     | 172  |               | 0x7E003000     | timer clock          | ?            |                         |
| UART      | 183  | IBRD/FBRD     | 0x7E201028/2c  | UART reference clock | ?            |                         |
| Timer ARM |      |               | 0x7E00B000     | core_clock           | ?            |                         |
| USB       |      |               |                |                      | ?            |                         |

## Notes

- UART1, SPI and I2C derive their clocks from the core clock
- [General purpose clock generator](https://www.tablix.org/~avian/blog/archives/2018/02/notes_on_the_general_purpose_clock_on_bcm2835/)
- [Clock tree](https://elinux.org/The_Undocumented_Pi#Clocks)
- [Code example](https://github.com/BrianSidebotham/arm-tutorial-rpi/blob/master/part-5/armc-014/armc-014.c)

