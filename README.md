# Raspberry Pi bare-metal

This repository contains the code for libraries and projects implemented on a bare-metal Raspberry Pi. This repository is a collection of projects and not a tutorial.However, I documented each project as best I could to make them useful as starting points for other uses. I recommend reviewing the tutorials listed under the resources section below, where listed them in an ascending order of complexity.  

> The code in this repository targets Raspberry Pi model B, and Pi Zero or ZeroW.

## ARM documentation

- ARM Architecture Reference Manual (ARM), Revision: DDI 0100I
- ARM1176JZF-S Technical Reference Manual (TRM), Revision: r0p7 DDI 0301H (ID012310)
- Broadcom BCM2835 ARM Peripherals (06 February 2012)

## config.txt

I found that [setting the ```config.txt``` file](https://www.raspberrypi.org/documentation/configuration/config-txt/README.md) in the SD card's boot directory can help smooth out some configuration settings even when using the RPi in bare metal fashion. My ```config.txt``` file looks like this:  

```
# ---- Base settings for binary file loads ----
startfile=start.elf
fixupfile=fixup.dat
# ---- Video ----
# sdtv_aspect added to stretch view on Dell monitor
sdtv_aspect=1
```

I'm using an early model Raspberry Pi Model B and a Pi-Zero. The first lines are probably redundant, but I found that the last entry with ```sdtv_aspect=1``` properly stretches the video display to fit the full monitor without the left and right side black border lines. Other values may be needed (or none at all) with other monitor models or aspect-sizes.  

The memory split is the default standard of 64MB for the VC and 448 for the ARM, changing this ration can be done with ```gpu_mem=<size>```.

## Directories

- [samples](samples/README.md): contained a minimal set of C and assembly code files for a minimal implementation and project starting point.
- [lib](lib/README.md): libraries.
- [include](lib/include): library include files. The tool chain include files are referenced automatically by the tool chain.
- [doc](doc/): some extra material on various bare metal programming topics.

## Resources

- [Tutorial by Brian Sidebotham](https://github.com/BrianSidebotham/arm-tutorial-rpi)
- [Tutorial by David Welch](https://github.com/dwelch67/raspberrypi)
- [Tutorial by Brian Widdas](https://github.com/brianwiddas/pi-baremetal)
- [Tutorial by University of Cambridge](https://www.cl.cam.ac.uk/projects/raspberrypi/tutorials/os/)
- [Discussion forum](https://www.raspberrypi.org/forums/viewforum.php?f=72)
- [GCC ARM tool chain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)

