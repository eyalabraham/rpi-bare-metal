/********************************************************************
 * uart.c
 *
 *  UART interface module.
 *  This module interfaces with the PCXT Z80-SIO Ch.B serial port
 *
 *  June 1, 2019
 *
 *******************************************************************/

#include    <string.h>
#include    <stdio.h>

#include    "bcm2835.h"
#include    "auxuart.h"
#include    "gpio.h"
#include    "irq.h"

#include    "config.h"
#include    "uart.h"
#include    "util.h"

#define     UART_CMD_Q_LEN      10

#define     SLIP_END            0xC0        // start and end of every packet
#define     SLIP_ESC            0xDB        // escape start (one byte escaped data follows)
#define     SLIP_ESC_END        0xDC        // following escape: original byte is 0xC0 (END)
#define     SLIP_ESC_ESC        0xDD        // following escape: original byte is 0xDB (ESC)

/********************************************************************
 * Module static functions
 *
 */

/********************************************************************
 * Module globals
 *
 */
static  int         uart_module_initialized = 0;

#if UART_TEST_CMD == 0

static  cmd_q_t     command_queue[UART_CMD_Q_LEN];
static  int         cmd_in = 0;
static  int         cmd_out = 0;
static  int         cmd_count = 0;

#else

/********************************************************************
 *     Pre-fill test commands
 */

static  cmd_q_t     command_queue[UART_CMD_Q_LEN] =
{
    {0,  {0,9, 0,0,0,0,0}},
    {0,  {3,1,10,0,0,0,0}},
    {0,  {4,0,65,0,0,0,7}}
};

static  int         cmd_in = 0;
static  int         cmd_out = 0;
static  int         cmd_count = 3;

/********************************************************************/

#endif

/********************************************************************
 * uart_init()
 *
 * Initialize the UART and GPIO subsystems of BCM2835.
 * Failure to initialize any of the above three IO subsystems
 * will result in closing all open IO devices and exiting with an error.
 *
 *  param:  none
 *  return: 0 if no error,
 *         -1 if error initializing any subcomponent or library
 *
 */
int uart_init(void)
{
    /* Initialize RTS GPIO pin
     */
    bcm2835_gpio_fsel(RPIB_ACT_LED, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(UART_RTS, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_set(UART_RTS);

    /* Initialize UART1 with receive interrupts
     */
    disable();

        irq_init();
        bcm2835_auxuart_init(UART_BAUD, 0, 0, AUXUART_ENA_RX_IRQ);

    enable();

    debug(DB_INFO, "Initialized UART1 (Auxiliary UART)\n");

    uart_module_initialized = 1;

    return 0;
}

/********************************************************************
 * uart_get_cmd()
 *
 *  Check command queue for pending data and return pointer to data or 0 if none.
 *
 *  param:  none
 *  return: 0 if no commands, queue empty,
 *          pointer to next command
 */
cmd_q_t* uart_get_cmd(void)
{
    cmd_q_t* command;

    if ( cmd_count )
    {
        command = &command_queue[cmd_out];
        cmd_count--;
        cmd_out++;
        if ( cmd_out == UART_CMD_Q_LEN )
            cmd_out = 0;

        return command;
    }

    return 0;
}

/********************************************************************
 * uart_recv_cmd()
 *
 *  Check UART and add incoming commands to command queue.
 *  UART polling function to be called periodically.
 *  Function tries to assemble a complete command and exit when
 *  one command has been assembled or a read time-out has occurred.
 *
 *  param:  none
 *  return: Number of commands received
 *         -1 if error
 */
int uart_recv_cmd(void)
{
    int             read_result;
    int             commands = 0;

    static uint8_t  c;

    static int      cmd[(sizeof(cmd_param_t)/sizeof(int))] = {0};
    static int      slip_esc_received = 0;
    static int      count = 0;
    static int      done_cmd_packet = 0;

    if ( !uart_module_initialized )
    {
        debug(DB_ERR, "%s: UART is not initialized (line:%d)\n", __FUNCTION__, __LINE__);
        return -1;
    }

    /* collect bytes from the serial stream into a command sequence
     * bytes are received framed as a SLIP packet
     */
    uart_rts_active();

    while ( 1 )
    {
        read_result = bcm2835_auxuart_rx_byte(&c);

/*
        if ( read_result == 1 )
        {
            debug(DB_VERBOSE, "c=%d\n", c);
        }
*/

        // exit if nothing read or time out
        if ( read_result == 0 )
        {
            break;
        }
        // first check if previous character was a SLIP ESC
        else if ( slip_esc_received )
        {
            slip_esc_received = 0;
            if ( c == SLIP_ESC_END )
            {
                c = SLIP_END;
            }
            else if ( c == SLIP_ESC_ESC )
            {
                c = SLIP_ESC;
            }
        }
        // handle packet delimiter
        else if ( c == SLIP_END )
        {
            // back-to-back END
            if ( count == 0 )
            {
                continue;
            }
            // command bytes received and packet is done
            else
            {
                done_cmd_packet = 1;
                count = 0;
                break;
            }
        }
        // handle SLIP escape in the byte stream
        else if ( c == SLIP_ESC )
        {
            slip_esc_received = 1;
            continue;
        }
        // handle full packet with no delimiter
        else if ( count == (sizeof(cmd_param_t)/sizeof(int)) )
        {
            debug(DB_ERR, "%s: invalid command frame; discarding\n", __FUNCTION__);
            count = 0;
            break;
        }

        cmd[count] = c;
        count++;
    }

    uart_rts_not_active();

    /* If there is a complete command packet ready
     * process it
     */
    if ( done_cmd_packet )
    {
        done_cmd_packet = 0;

        // check if there is room in the queue for this command
        if ( cmd_count == UART_CMD_Q_LEN )
        {
            count = 0;
            debug(DB_ERR, "%s: command buffer overflow; discarding\n", __FUNCTION__);
            return -1;
        }

        // copy new command to the queue
        commands = 1;
        command_queue[cmd_in].queue = (int)(((uint32_t)cmd[0] >> 6) & 0x03);
        memcpy(&command_queue[cmd_in].cmd_param, &cmd, sizeof(cmd_param_t));

        debug(DB_VERBOSE, "uart_recv_cmd(): [%d] %3d | %3d %3d %3d %3d %3d %3d\n",
                          command_queue[cmd_in].queue,
                          command_queue[cmd_in].cmd_param.cmd,
                          command_queue[cmd_in].cmd_param.b1,
                          command_queue[cmd_in].cmd_param.b2,
                          command_queue[cmd_in].cmd_param.b3,
                          command_queue[cmd_in].cmd_param.b4,
                          command_queue[cmd_in].cmd_param.b5,
                          command_queue[cmd_in].cmd_param.b6);

        memset(&cmd, 0, sizeof(cmd_param_t));
        cmd_count++;
        cmd_in++;
        if ( cmd_in == UART_CMD_Q_LEN )
            cmd_in = 0;
    }

    return commands;
}


/********************************************************************
 * uart_send()
 *
 *  Send a data byte to host PC/XT.
 *
 *  param:  data byte to send
 *  return: none
 */
void uart_send(uint8_t byte)
{
    if ( !uart_module_initialized )
    {
        debug(DB_ERR, "%s: UART is not initialized (line:%d)\n", __FUNCTION__, __LINE__);
        return;
    }

    bcm2835_auxuart_putchr(byte);
}

/********************************************************************
 * uart_rts_active()
 *
 *  RTS line to '0'
 *
 *  param:  none
 *  return: none
 *
 */
void uart_rts_active(void)
{
    if ( uart_module_initialized )
    {
        bcm2835_gpio_clr(UART_RTS);
    }
}

/********************************************************************
 * uart_rts_not_active()
 *
 *
 *  RTS line to '1'
 *
 *  param:  none
 *  return: none
 *
 */
void uart_rts_not_active(void)
{
    if ( uart_module_initialized )
    {
        bcm2835_gpio_set(UART_RTS);
    }
}
