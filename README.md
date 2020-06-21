# Raspberry Pi bare-metal

This repository contains the code for libraries and projects implemented on a bare-metal Raspberry Pi. This repository is a collection of projects and not a tutorial.However, I documented each project as best I could to make them useful as starting points for other uses. I recommend reviewing the tutorials listed under the resources section below, where listed them in an ascending order of complexity.  

> The code in this repository targets Raspberry Pi model B, and Pi Zero or ZeroW.

## ARM documentation

- ARM Architecture Reference Manual (ARM), Revision: DDI 0100I
- ARM1176JZF-S Technical Reference Manual (TRM), Revision: r0p7 DDI 0301H (ID012310)
- Broadcom BCM2835 ARM Peripherals (06 February 2012)

## Resources

- [Tutorial by Brian Sidebotham](https://github.com/BrianSidebotham/arm-tutorial-rpi)
- [Tutorial by David Welch](https://github.com/dwelch67/raspberrypi)
- [Tutorial by Brian Widdas](https://github.com/brianwiddas/pi-baremetal)
- [Tutorial by University of Cambridge](https://www.cl.cam.ac.uk/projects/raspberrypi/tutorials/os/)
- [Discussion forum](https://www.raspberrypi.org/forums/viewforum.php?f=72)
- [GCC ARM tool chain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)

## Directories

- [samples](samples/README.md): contained a minimal set of C and assembly code files for a minimal implementation and project starting point.
- [lib](lib/README.md): libraries.
- [include](lib/include): library include files. The tool chain include files are referenced automatically by the tool chain.
- [doc](doc/): some extra material on various bare metal programming topics.
