/*
 * fb.c
 *
 *  Test the use of mailbox to configure the video frame buffer and display
 *  a Mandelbrot set fractal graphics.
 *  Based on 'armc-016.c' by Brian Sidebotham's RPi bare-metal tutorial.
 *  Requires a serial terminal set to 9600,N,1 and a connection to a video monitor.
 *
 */

#include    <stdint.h>
#include    <string.h>
#include    <math.h>

#include    "printf.h"
#include    "bcm2835.h"
#include    "auxuart.h"
#include    "timer.h"
#include    "mailbox.h"

/* -----------------------------------------
   Local definitions
----------------------------------------- */
#define     ESC             27

#define     DISP_WIDTH      640
#define     DISP_HEIGHT     480

#define     FRACTAL_COLORS  16

#define     MAX_ITER        100
#define     FRAC_X_MIN     -2.5
#define     FRAC_X_MAX      1.0
#define     FRAC_Y_MIN     -1.0
#define     FRAC_X_MAX      1.0

/* -----------------------------------------
   Module static functions
----------------------------------------- */
void halt(char *msg);
void dump_tag(mailbox_tag_property_t *mb_tag);

/* -----------------------------------------
   Module globals
----------------------------------------- */
//uint32_t cga_palette_rgb[] =
//{
//        0x00000000,
//        0x00000080,
//        0x00008000,
//        0x00008080,
//        0x00800000,
//        0x00800080,
//        0x00808000,
//        0x00C0C0C0,
//        0x00808080,
//        0x000000FF,
//        0x0000FF00,
//        0x0000FFFF,
//        0x00FF0000,
//        0x00FF00FF,
//        0x00FFFF00,
//        0x00FFFFFF
//};

/* Load this palette for 8-bpp color depth.
 * The palette is in BGR format, and 'set pixel order' does not affect
 * palette behavior.
 * Palette source: https://en.wikipedia.org/wiki/Web_colors#HTML_color_names
 */
uint32_t cga_palette_bgr[] =
{
        0x00000000,
        0x00800000,
        0x00008000,
        0x00808000,
        0x00000080,
        0x00800080,
        0x00008080,
        0x00C0C0C0,
        0x00808080,
        0x00FF0000,
        0x0000FF00,
        0x00FFFF00,
        0x000000FF,
        0x00FF00FF,
        0x0000FFFF,
        0x00FFFFFF
};

volatile uint8_t       *fb = NULL;
mailbox_tag_property_t *mp;

uint32_t    buffer_address;
int         width, height, bpp, pitch;
int         pixel_offset;
int         i;

/* Fractal variable
 */
float   scale_x, scale_y;
float   temp, x, y, x_ini, y_ini;
int     iteration, hx, hy, color;
int     max_iter = -1, min_iter = 9999;

/* Values in the iteration-to-color mapping arrays below, iterations[],
 * were obtained after graphing a histogram of the iteration counts
 * with the code related to the bins[] array.
 */
//int     iterations[FRACTAL_COLORS] = {MAX_ITER, 15, 5, 0};   // keep MAX_ITER and '0', change all other numbers
int     iterations[FRACTAL_COLORS] =
{
        MAX_ITER,
        81,
        64,
        47,
        30,
        20,
        16,
        12,
        8,
        6,
        5,
        4,
        3,
        2,
        1,
        0              // keep MAX_ITER and '0', change all other numbers
};

//int     bins[MAX_ITER+1];

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
    /* Initialize the IO system
     */
    bcm2835_auxuart_init(BAUD_9600, 0, 0, AUXUART_DEFAULT);

    printf("%c[2J", ESC);
    printf("RPi bare-metal %s %s\n", __DATE__, __TIME__ );
    printf("CPU ID: 0x%x\n", machid);
    printf("bcm2835 Frame buffer with Mailbox interface\n\n");

    /* Configure the video frame buffer.
     */
    bcm2835_mailbox_init();

    /* ***
     * "If an allocate buffer tag is omitted when setting parameters,
     *  then no change occurs unless it can be accommodated without changing
     *  the buffer base or size."
     */
    bcm2835_mailbox_add_tag(TAG_FB_ALLOCATE, 4);

    bcm2835_mailbox_add_tag(TAG_FB_SET_PHYS_DISPLAY, DISP_WIDTH, DISP_HEIGHT);
    bcm2835_mailbox_add_tag(TAG_FB_SET_VIRT_DISPLAY, DISP_WIDTH, DISP_HEIGHT);
    bcm2835_mailbox_add_tag(TAG_FB_SET_DEPTH, 8);
    bcm2835_mailbox_add_tag(TAG_FB_SET_PALETTE, 0, 16, (uint32_t)cga_palette_bgr);
    bcm2835_mailbox_add_tag(TAG_FB_GET_PITCH);
    bcm2835_mailbox_add_tag(TAG_FB_GET_PALETTE);

    if ( !bcm2835_mailbox_process() )
        halt("Mailbox call failed.");

    mp = bcm2835_mailbox_get_property(TAG_FB_ALLOCATE);
    if ( mp )
    {
        printf("Frame buffer: VC base 0x%08x, %u[Bytes]\n", mp->values.fb_alloc.param1,
                                                            mp->values.fb_alloc.param2);
        buffer_address = mp->values.fb_alloc.param1;
    }
    else
        halt("TAG_FB_ALLOCATE");

    mp = bcm2835_mailbox_get_property(TAG_FB_SET_VIRT_DISPLAY);
    if ( mp )
    {
        printf("Virtual: width %u, height %u pixels\n", mp->values.fb_set.param1,
                                                        mp->values.fb_set.param2);
    }
    else
        halt("TAG_FB_SET_VIRT_DISPLAY");

    mp = bcm2835_mailbox_get_property(TAG_FB_SET_PHYS_DISPLAY);
    if ( mp )
    {
        width = mp->values.fb_set.param1;
        height = mp->values.fb_set.param2;
        printf("Display: width %u, height %u pixels\n", width, height);
    }
    else
        halt("TAG_FB_SET_PHYS_DISPLAY");

    mp = bcm2835_mailbox_get_property(TAG_FB_SET_DEPTH);
    if ( mp )
    {
        bpp = mp->values.fb_set.param1;
        printf("         depth %u bits-per-pixel\n", bpp);
    }
    else
        halt("TAG_FB_SET_DEPTH");

    mp = bcm2835_mailbox_get_property(TAG_FB_GET_PITCH);
    if ( mp )
    {
        pitch = mp->values.fb_get.param1;
        printf("         pitch %u bytes-per-line\n", pitch);
    }
    else
        halt("TAG_FB_GET_PITCH");

//    mp = bcm2835_mailbox_get_property(TAG_FB_GET_PALETTE);
//    mp = bcm2835_mailbox_get_property(TAG_FB_SET_PALETTE);
//    if ( mp )
//    {
//        dump_tag(mp);
//    }
//    else
//        halt("TAG_FB_*_PALETTE");

    /* For some reason modifying the address as described here:
     * https://github.com/raspberrypi/firmware/wiki/Accessing-mailboxes
     * has no effect on ability to access the buffer.
     */
    fb = (uint8_t*) (buffer_address);

    /* Draw color bars
     */
    for (i = 0; i < 2; i++)
    {
        for (color = 0; color < 8; color++)
        {
            for (hy = 0; hy < (height/2); hy++ )
            {
                for (hx = 0; hx < (width/8); hx++)
                {
                    pixel_offset = (hx * (bpp/8)) + (color * (width/8)) +
                                   (hy * pitch) + ((height/2) * pitch * i);
                    fb[pixel_offset] = (uint8_t)(color + (i * 8));
                }
            }
        }
    }

    bcm2835_st_delay(10000000);

    /* Draw fractal
     */
    scale_x = (fabsf(FRAC_X_MIN) + fabsf(FRAC_X_MAX)) / (width);
    scale_y = (fabsf(FRAC_Y_MIN) + fabsf(FRAC_X_MAX)) / (height);

    for (hy = 1; hy < height; hy++)
    {
        for (hx = 1; hx < width; hx++)
        {
            x_ini = scale_x * hx + FRAC_X_MIN;
            y_ini = FRAC_X_MAX - scale_y * hy;
            x = 0.0; y = 0.0;

            for ( iteration = 0; iteration < MAX_ITER; iteration++ )
            {
                temp = x*x-y*y+x_ini;
                y = 2.0*x*y+y_ini;
                x = temp;
                if ( (x*x+y*y) > 4.0)
                    break;
            }

//            bins[iteration] += 1;

            for ( color = 0; color < FRACTAL_COLORS; color++ )
            {
                if ( iteration < iterations[color] )
                    continue;
                else
                    {
                        pixel_offset = (hx * (bpp/8)) + (hy * pitch);
                        fb[pixel_offset] = (uint8_t)color;
                        break;
                    }
            }
        }
    }

//    for (i = 0; i <= MAX_ITER; i++)
//        printf("%2d, %d\n", i, bins[i]);

    halt("Done.");
}

/*------------------------------------------------
 * halt()
 *
 *  Print message and halt in endless loop
 *
 *  param:  Message
 *  return: none
 */
void halt(char *msg)
{
    printf("%s\nHalted.\n", msg);

    while (1)
    {
        /* Do nothing */
    }
}

/*------------------------------------------------
 * dump_mb_message()
 *
 *  Dump print mailbox message tag content.
 *
 *  param:  Pointer to mailbox tag area
 *  return: none
 */
void dump_tag(mailbox_tag_property_t *mb_tag)
{
    uint32_t   *pvalues;
    int         i, values;

    if ( mb_tag->req_resp_status & 0x80000000 )
    {
        values = (mb_tag->req_resp_status & 0x7fffffff) / 4;
        printf("Response tag.\n");
    }
    else
    {
        values = mb_tag->values_length / 4;
        printf("Request tag.\n");
    }

    printf("Tag    0x%08x\n", mb_tag->tag);
    printf("Length 0x%08x\n", mb_tag->values_length);
    printf("Status 0x%08x\n", mb_tag->req_resp_status);
    printf("Values\n");
    pvalues = (uint32_t*) &(mb_tag->values);
    for (i = 0; i < values; i++)
        printf("   %4d) 0x%08x\n", i, pvalues[i]);
}

/*------------------------------------------------
 * _putchar()
 *
 *  Low level character output/stream for printf()
 *
 *  param:  character
 *  return: none
 */
void _putchar(char character)
{
    if ( character == '\n')
        bcm2835_auxuart_putchr('\r');
    bcm2835_auxuart_putchr(character);
}
