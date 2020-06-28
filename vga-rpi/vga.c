/********************************************************************
 * vga.c
 *
 *  Main module for PC/XT graphics card emulation for VGA
 *  on Raspberry Pi bare metal.
 *
 *
 *  June 21, 2020
 *
 *******************************************************************/

#include    <stdint.h>

#include    "config.h"
#include    "util.h"
#include    "fb.h"
#include    "uart.h"

/********************************************************************
 * Module globals
 *
 */
cmd_q_t* command_q = 0;

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
    /* TODO Set debug level
     */
    debug_lvl(0);

    /* Start emulation loop
     */
    uart_init();
    debug(DB_VERBOSE, "Starting VGA emulator.\n");

    if ( fb_init(VGA_DEF_MODE) == 0 )
    {
        uart_rts_active();      // this signals a ready state to the PCXT

        /* VGA card emulator processing loop
         */
        while (1)
        {
            command_q = uart_get_cmd();

            if ( command_q )
            {
                /* Handle VGA emulation
                 */
                if ( command_q->queue == UART_Q_VGA )
                {
                    fb_emul(&(command_q->cmd_param));
                }
                /* Handle queue #1
                 */
                else if ( command_q->queue == UART_Q_OTHER1 )
                {
                }
                /* Handle queue #2
                 */
                else if ( command_q->queue == UART_Q_OTHER2 )
                {
                }
                /* Handle system commands
                 */
                else if ( command_q->queue == UART_Q_SYSTEM )
                {
                    if ( command_q->cmd_param.cmd == UART_CMD_ECHO )
                    {
                        debug(DB_INFO, "echo reply\n");
                        echo_reply();
                    }
                }
                else if ( command_q->queue == UART_Q_ABRT )
                {
                    debug(DB_ERR, "aborting test.\n");
                    break;
                }
            }

            fb_cursor_blink();

            uart_recv_cmd();
        }
    }
    else
    {
        debug(DB_ERR, "%s: frame buffer and/or GPIO initialization failed\n", __FUNCTION__);
    }

    halt("Halting.");
}
