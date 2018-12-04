/*********************************************************************
 *
 *     UART access routines for C32 and XC32
 *
 *********************************************************************
 * FileName:        UART.c
 * Dependencies:    Hardware UART module
 * Processor:       PIC18, PIC24F, PIC24H, dsPIC30F, dsPIC33F
 * Compiler:        Microchip C32 v
 *                  Microchip XC32 v1.20 or higher
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * Copyright (C) 2002-2009 Microchip Technology Inc.  All rights
 * reserved.
 *
 * Microchip licenses to you the right to use, modify, copy, and
 * distribute:
 * (i)  the Software when embedded on a Microchip microcontroller or
 *      digital signal controller product ("Device") which is
 *      integrated into Licensee's product; or
 * (ii) ONLY the Software driver source files ENC28J60.c, ENC28J60.h,
 *		ENCX24J600.c and ENCX24J600.h ported to a non-Microchip device
 *		used in conjunction with a Microchip ethernet controller for
 *		the sole purpose of interfacing with the ethernet controller.
 *
 * You should refer to the license agreement accompanying this
 * Software for additional information regarding your rights and
 * obligations.
 *
 *
 * Author               Date   		Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Howard Schlunder	4/04/06		Copied from dsPIC30 libraries
 * Howard Schlunder	6/16/06		Added PIC18
 * Richard Wall         11/26/2013  Removed all but PIC32
 *                              Added initialize_uart1 to used
 *                              peripheral library functions
 *                              Removed references to all but PIC32
********************************************************************/
#define __UART_C

#include "TCPIPConfig.h"
#include "HardwareProfile.h"

#if defined(STACK_USE_UART)

#include "TCPIP Stack/TCPIP.h"

/* initialize_comm FUNCTION DESCRIPTION *************************************
 * SYNTAX:          void initialize_comm(unsigned int baud, int parity);
 * KEYWORDS:        UART, initialization, parity
 * DESCRIPTION:     Initializes UART1 comm port for specified baud rate using
 *                  the assigned parity
 * PARAMETER 1:     integer Baud rate
 * PARAMETER 1:     integer (parity, NO_PARITY, ODD_PARITY, or EVEN_PARITY)
 * RETURN VALUE:    None
 *
 * NOTES:           9 bit mode MARK or SPACE parity is not supported
 * END DESCRIPTION **********************************************************/
void initialize_uart1(unsigned int baud, int parity)
{
unsigned int BRG;

    BRG=(unsigned short)(((float)GetPeripheralClock() / ((float)4 * \
                          (float) baud))-(float)0.5);
    switch(parity)
    {
        case NO_PARITY:
            OpenUART1( (UART_EN | UART_BRGH_FOUR | UART_NO_PAR_8BIT),
                        (UART_RX_ENABLE | UART_TX_ENABLE) , BRG );
            break;
        case ODD_PARITY:
            OpenUART1( (UART_EN | UART_BRGH_FOUR | UART_ODD_PAR_8BIT),
                        (UART_RX_ENABLE | UART_TX_ENABLE) , BRG );
            break;
        case EVEN_PARITY:
            OpenUART1( (UART_EN | UART_BRGH_FOUR | UART_EVEN_PAR_8BIT),
                        (UART_RX_ENABLE | UART_TX_ENABLE) , BRG );
            break;
    }
    printf("\n\rchipKIT Pro MX7 Serial Port 1 ready\n\r");
}

/* _mon_putc FUNCTION DESCRIPTION ******************************************
 * SYNTAX:          void _mon_putc(char c);
 * KEYWORDS:        printf, console, monitor
 * DESCRIPTION:     Sets up serial port to function as console for printf.
 *                  Used only by system.
 * PARAMETER 1:     Character to send to monitor
 * RETURN VALUE:    None
 * NOTES:           This function will block until space is available
 *                  in the transmit buffer
 * END DESCRIPTION **********************************************************/
void _mon_putc(char c)
{
    while(BusyUART1());
    WriteUART1((unsigned int) c);
} /* End of _mon_putc */

/* putU1 FUNCTION DESCRIPTION ********************************************
 * SYNTAX:          int putU1( int c);
 * KEYWORDS:        UART, character
 * DESCRIPTION:     Waits while UART1 is busy (buffer full) and then sends a
 *                  single byte to UART1
 * PARAMETER 1:     character to send
 * RETURN VALUE:    character sent
 * NOTES:           This function will block until space is available
 *                  in the transmit buffer
 * END DESCRIPTION **********************************************************/
int putcU1( int c)
{
    while(BusyUART1());
    WriteUART1((unsigned int) c);
    return c;
} /* End of putU1 */

/* getU1 FUNCTION DESCRIPTION ********************************************
 * SYNTAX:          int getU1( char *ch);
 * KEYWORDS:        character, get
 * DESCRIPTION:     Checks for a new character to arrive to the UART1 serial port.
 * PARAMETER 1:     character pointer to character variable
 * RETURN VALUE:    TRUE = new character received
 *                  FALSE = No new character
 * NOTES:           This function does not block for no character received
 * END DESCRIPTION ********************************************************/
int getcU1( char *ch)
{
    if( !DataRdyUART1())	/* wait for new char to arrive */
        return FALSE;           /* Return new data not available flag */
    else
    {
        *ch = ReadUART1();	/* read the char from receive buffer */
        return TRUE;		/* Return new data available flag */
    }
}/* End of getU1 */

/* putsU1 FUNCTION DESCRIPTION ********************************************
 * SYNTAX:          int putsU1( const char *s);
 * KEYWORDS:        UART, string
 * DESCRIPTION:     Sends a NULL terminates text string to UART1 with
 *                  CR and LF appended
 * PARAMETER 1:     pointer to text string
 * RETURN VALUE:    Logical TRUE
 * NOTES:           This function will block until space is available
 *                  in the transmit buffer
 * END DESCRIPTION **********************************************************/
int putsU1( const char *s)
{
    putsUART1(s);
    putcUART1( '\r');
    putcUART1( '\n');
    return 1;
} /* End of putsU1 */

/* getstrU1 FUNCTION DESCRIPTION ********************************************
 * SYNTAX:          int getstrU1( char *s, unsigned int len );
 * KEYWORDS:        string, get, UART
 * DESCRIPTION:     This function assembles a line of text until the number of
 *                  characters assembled exceed the buffer length or an ASCII
 *                  CR control character is received.  This function echo each
 *                  received character back to the UART. It also implements a
 *                  destructive backspace. ASCII LF control characters are
 *                  filtered out.  The returned string has the CR character
 *                  removed and a NULL character appended to terminate the text
 *                  string.
 * PARAMETER 1:     character pointer to string
 * PARAMETER 2:     integer maximum string length
 * RETURN VALUE:    TRUE = EOL signaled br receiving return character
 *                  FALSE = waiting for end of line
 * NOTES:           It is presumed that the buffer pointer or the buffer length
 *                  does not change after the initial call asking to
 *                  receive a new line of text.  This function does not block
 *                  for no character received. A timeout can be added to this
 *                  to free up resource. There is no way to restart the function
 *                  after the first call until a EOL has been received.  Hence
 *                  this function has denial of service security risks.
 * END DESCRIPTION **********************************************************/
BYTE ReadStringUART( char *s, WORD len)
{
static int eol = TRUE;  /* End of input string flag*/
static unsigned int buf_len;
static char *p1;        /* copy #1 of the buffer pointer */
static char *p2;        /* copy #2 of the buffer pointer */
char ch;                /* Received new character */

    if(eol)		/* Initial call to function - new line */
    {			/* Make two copies of pointer - one for */
        p1 = s;		/* receiving characters and one for marking */
	p2 = s;		/* the starting address of the string.  The */
        eol = FALSE;	/* second copy is needed for backspacing. */
	buf_len = len;  /* Save maximum buffer length */
    }

    if(!(getcU1(&ch)))	/*  Check for character received */
    {
        return FALSE;	/* Bail out if not */
    }
    else
    {
        *p1 = ch;       /* Save new character in string buffer */
   	putcU1( *p1);   /* echo character */
	switch(ch)	/* Test for control characters */
	{
            case BACKSPACE:
                if ( p1>p2)
                {
                    putcU1( ' ');     /* overwrite the last character */
                    putcU1( BACKSPACE);
                    buf_len++;
                    p1--;            /* back off the pointer */
                }
                break;
            case '\r':              /* end of line, end loop */
                putcU1( '\n');      /* add line feed */
                eol = TRUE;         /* Mark end of line */
            	break;
            case '\n':              /* line feed, ignore it */
                break;
            default:
                p1++;               /* increment buffer pointer */
                buf_len--;          /* decrement length counter */
        }
    }
    if( buf_len == 0 || eol)    /* Check for buffer full or end of line */
    {
        *p1 = '\0';             /* add null terminate the string */
        return TRUE;            /* Set EOL flag */
    }
    return FALSE;               /* Not EOL */
} /* End of getstr */


#endif	//STACK_USE_UART
