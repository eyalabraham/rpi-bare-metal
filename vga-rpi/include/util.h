/********************************************************************
 * util.h
 *
 *  Utility and helper functions (debug print etc)
 *
 *  June 26, 2020
 *
 *******************************************************************/

#ifndef __util_h__
#define __util_h__

#define     DB_ERR      0
#define     DB_INFO     1
#define     DB_VERBOSE  2

/********************************************************************
 * Function prototypes
 *
 */
void debug_lvl(int);
int  debug(int, char *, ...);
void echo_reply(void);
void halt(char *);
void _putchar(char);

#endif  /* __util_h__ */
