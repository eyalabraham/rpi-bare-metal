# VGA display card based on Raspberry Pi

The [Video display card](https://sites.google.com/site/eyalabraham/pc-xt/video-display) mimics an MDA/CGA text and graphics cards on a standard VGA monitor. The emulated graphics card uses a Raspberry Pi (RPi) running custom software as a video generator. The software runs on a bare metal Raspberry Pi with no OS. The RPi will connect through its Auxiliary UART to the PC's Z80-SIO/2 USART to interface with the 8088 PCXT bus.  
This project is a bare metal implementation of the [same Linux-based](https://github.com/eyalabraham/vga-rpi) code. I probably should have first refactored the original project in a way that separates the Linux-dependent code from bare-metal, and isolated the implementation independent modules; maybe next time...

> Supports only Raspberry Pi model B and Zero

## Resources

- [RPi GPIO resorces](https://pinout.xyz/pinout/ground#)
- [BCM2835](https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf) and errata (https://elinux.org/BCM2835_datasheet_errata)
- Google search "Raspberry Pi bare metal"
- ARM1176JZF-S [Technical Reference Manual](http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0301h/index.html)
- [Z80-SIO USART](http://www.z80.info/zip/um0081.pdf)

## Schematics

- [Schematics GitHub URL](https://github.com/eyalabraham/schematics/tree/master/vga-rpi)

### RPi GPIO setup and mapping to Z80-SIO channel B

| Pin # | GPIO # | Function  | GPIO Alt |  Z80-SIO   |
|-------|--------|-----------|----------|------------|
| 8     |   14   | TxD1      | ALT5 / 0 |  RxDB p.29 |
| 10    |   15   | RxD1      | ALT5 / 0 |  TxDB p.26 |
| 36    |   16   | CTS1      | ALT5 / 3 |  RTSB p.24 |
| 11    |   17   | RTS1      | ALT5 / 3 |  CTSB p.23 |

## Software

The RPi software works in conjunction with the PC/XT [BIOS code](https://github.com/eyalabraham/new-xt-bios). The main component in the RPi software is the Frame Buffer on bare-metal.

### Emulated graphics cards and modes

 | mode | resolution | color      | test/graph | pages | card | emulated |
 |------|------------|------------|------------|-------|------|----------|
 | 0    | 40x25      | Monochrome | text       |  8    | CGA  |   no     |
 | 1    | 40x25      | 16 color   | text       |  8    | CGA  |   yes    |
 | 2    | 80x25      | 16 gray    | text       |  4    | CGA  |   no     |
 | 3    | 80x25      | 16 color   | text       |  4    | CGA  |   yes    |
 | 4    | 320x200    | 4 color    | graphics   |  1    | CGA  |   no     |
 | 5    | 320x200    | 4 color    | graphics   |  1    | CGA  |   no     |
 | 6    | 640x200    | Monochrome | graphics   |  1    | CGA  |   no     |
 | 7    | 80x25      | Monochrome | text       |  1    | MDA  |   yes    |
 | 8    | 720x350    | Monochrome | graphics   |  1    | HERC |   no     |
 | 9    | 1280x1024  | Monochrome | text (1)   |  1    | VGA  |   no     |

(1) This is a special mode for mon88, text 160x64

### Protocol for display control

PC/XT will translate INT 10h calls from the application into a set of bytes sent to the RPi. These bytes will form the control primitives that manage the display. The bytes will be sent as "packets" through Z80-SIO UART channel B that is connected to the RPi UART.
Each packet will be delimited by escape characters similar to the Serial IP protocol [SLIP](https://en.wikipedia.org/wiki/Serial_Line_Internet_Protocol), with END escape codes at the start and end of the command primitive packet. This is a simple and efficient way to frame these command packets.

The control packets include:

| Command (5)(10)   | Queue | cmd | byte.1              | byte.2          | byte.3        | byte.4    | byte.5  | byte.6     |
|-------------------|-------|-----|---------------------|-----------------|---------------|-----------|---------|------------|
| Set video mode    |  0    | 0   | Mode=0..8 see above | 0               | 0             | 0         | 0       | 0          |
| Set display page  |  0    | 1   | Page                | 0               | 0             | 0         | 0       | 0          |
| Cursor position   |  0    | 2   | Page                | 0               | col=0..79(39) | row=0..24 | 0       | 0          |
| Cursor size/mode  |  0    | 3   | Top scan line 11)   | Bottom scan line| 0             | 0         | 0       | 0          |
| Put character (1) |  0    | 4   | Page                | char code       | col=0..79(39) | row=0..24 | 0       | Attrib.(2) |
| Get character (6) |  0    | 5   | Page                | 0               | col=0..79(39) | row=0..24 | 0       | 0          |
| Put character (7) |  0    | 6   | Page                | char code       | col=0..79(39) | row=0..24 | 0       | 0          |
| Scroll up (4)     |  0    | 7   | Rows                | T.L col         | T.L row       | B.R col   | B.R row | Attrib.(2) |
| Scroll down (4)   |  0    | 8   | Rows                | T.L col         | T.L row       | B.R col   | B.R row | Attrib.(2) |
| Put pixel         |  0    | 9   | Page                | Pixel color (3) |       16-bit column       |     16-bit row       |
| Get pixel (8)     |  0    | 10  | Page                | 0               |       16-bit column       |     16-bit row       |
| Set palette       |  0    | 11  | palette/color       | palette ID      | 0             | 0         | 0       | 0          |
| Clear screen      |  0    | 12  | Page                | 0               | 0             | 0         | 0       | Attrib.(2) |
| Echo (9)          |  3    | 63  | 1                   | 2               | 3             | 4         | 5       | 6          |

(1) Character is written to specified {col}{row} position  
(2) Attribute: Attribute byte will be decoded per video mode  
(3) XOR-ed with current pixel if bit.7=1  
(4) Act on active page  
(5) PC/XT can **not** send partial commands, any trailing bytes should be padded with '0'  
(6) Return data format: two bytes {character}{attribute}  
(7) Same at command #4, but use existing attribute  
(8) Return data format: one byte {color_code}  
(9) Return data format: six bytes {6}{5}{4}{3}{2}{1}  
(10) Two high order bits are command queue: '00' VGA emulation, '01' tbd, '10' tbd, '11' system  
(11) A value of 2000h turns cursor off.  

### INT 10h mapping to display control commands

Functions not listed below will be handled by BIOS, and not transferred to the display controller.
The display controller will keep cursor position but will not track movement through writes; cursor reposition will be done by BIOS with command #2.  
Put Character commands #4 and #6 provide direct character placement parameters, these do not change display controller's stored cursor position. Only command #2 can change display controller's stored cursor position. Therefore, INT 10h functions 0Eh and 13h will output the characters with commands #4 and #6 and then provide one cursor reposition command #2. For visual effect, cursor 'off' and 'on' with command #3 can be used. Command #11 can be used with scroll commands if the text screen needs to be cleared.

| INT       | Function                                   | Command |
|-----------|--------------------------------------------|---------|
| INT 10,0  | Set video mode                             | #0      |
| INT 10,1  | Set cursor type/size                       | #3      |
| INT 10,2  | Set cursor position                        | #2      |
| INT 10,5  | Select active display page                 | #1      |
| INT 10,6  | Scroll active page up                      | #7      |
| INT 10,7  | Scroll active page down                    | #8      |
| INT 10,8  | Read character and attribute at cursor     | #5      |
| INT 10,9  | Write character(s) and attribute at cursor | #4      |
| INT 10,A  | Write character(s) at current cursor       | #6      |
| INT 10,C  | Write graphics pixel at coordinate         | #9      |
| INT 10,D  | Read graphics pixel at coordinate          | #10     |
| INT 10,E  | Write text in teletype mode                | #6,#2   |
| INT 10,13 | Write string (BIOS after 1/10/86)          | #4,#2   |

### Files

- ```vga.c``` main module and emulator control loop
- ```fb.c``` frame buffer and graphics emulation
- ```uart.c``` UART IO driver
- ```util.c``` utility and helper functions (debug print etc)
- ```include/iv8x16u.h``` 8x16 font bitmap definition [bitmap font source](https://github.com/farsil/ibmfonts) for code page 437 characters
- ```include/ic8x8u.h```  8x8 font bitmap definition [bitmap font source](https://github.com/farsil/ibmfonts) for code page 437 characters
- ```include/config.h``` compile time module configuration

