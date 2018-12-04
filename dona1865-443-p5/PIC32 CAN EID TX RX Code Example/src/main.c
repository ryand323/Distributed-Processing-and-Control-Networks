/**********************************************************************
 * FileName:        main.c
 * Dependencies:    Header (.h) files if applicable, see below
 * Processor:       PIC32
 * Compiler:        MPLAB® XC32
 * IDE              Microchip MPLAB (R) X 1.80 or higher
 * Author:          Richard W. Wall
 * Date:            June 26, 2013
 * Revised:         October 3, 2013
 * 
 * This code example runs on a Digilent Cerebot MX7ck board with a 8Mhz crystal
 * The CAN1 and CAN2 connectors on the Cerebot MX7ck board are connected to
 * each other to form a 2 node CAN bus. The code in this reference design
 * is based around Microchip "PIC32 CAN TX RX Code Example"
 *
 * The code example shows how to transmit and receive standard ID messages
 * using the PIC32 CAN modules and the PIC32 Peripheral Library. In this case,
 * CAN1 sends a Standard ID message to CAN1 and CAN2 replies back with a
 * Standard ID message.
 *
 * Every 1 second, CAN1 will send a message to CAN2 and toggle LEDA. When CAN2
 * receives the message, it will toggle LEDB and will then send a message to
 * CAN1 as well as toggle LEDB.  When CAN1 receives the message, it will
 * toggle LEDC. Hence all LEDs will toggle every second.
 * Configuration Bit settings - See config_bits.h
 * Each CAN module uses 2 Channels (Channel 0  and Channel 1). Each channel
 * is configured to be 8 messages deep. Channel 0 is configured for Transmit
 * and Channel 1 is configured for receive. CAN module configuration code is in
 * CANFunctions.c
 *
 * Note the size of each Channel area. It is 2 (Channels) * 8 (Messages Buffers)
 * 16 (bytes/per message buffer) bytes. Each CAN module should have its own
 * message area.
 *
 ***********************************************************************/

#include <plib.h>

#include "chipKIT_Pro_MX7.h"
#include "sw_timer.h"

#include "GenericTypeDefs.h"
#include "CANFunctions.h"

static void SetupHardware( void );

#define ONE_SECOND  1000
int main(void)
{
    SetupHardware();

/* Functions are defined in CANFunctions.c */
    CAN1Init();
    CAN2Init(); 

    while(1)
    {
/* In this loop, we wait till we have 1 second tick. Each second CAN1 will
 * send a message to CAN2 and toggles LEDA. When CAN2 receives this message
 * it toggles LEDD. It then sends a message to CAN1 and toggles LEDB. When
 * CAN1 receives this message it toggles LEDC. If one second is up then
 * CAN1 sends a message to CAN2 to toggle LEDA and the process repeats. */

        if(PeriodMs(0) == 0)
        {
            CAN1TxSendLEDMsg();	/* Function is defined in CANFunctions.c */
            PeriodMs(ONE_SECOND);
        }

/* CAN2RxMsgProcess will check if CAN2 has received a message from CAN1 and
 * will toggle LEDD. It will send a message to CAN1 to toggle LEDB. */

        CAN2RxMsgProcess();     /* Function is defined in CANFunctions.c */

/* CAN1RxMsgProcess() will check if CAN1  has received a message from CAN2 and
 * will toggle LEDC. */

        CAN1RxMsgProcess();     /* Function is defined in CANFunctions.c */
    }
}

/* SetupHardware Function Description ****************************************
 * SYNTAX:          static void SetupHardware( void );
 * KEYWORDS:        System, initialization, hardware
 * DESCRIPTION:     Initializes chipKIT Pro MX7 I/O ports for common hardware
 * PARAMETERS:      None
 * RETURN VALUE:    None
 * NOTES:           The following lines of code are required regardless of what
 *                  other hardware setup is necessary.
 * END DESCRIPTION ************************************************************/
static void SetupHardware( void )
{
    chipKIT_PRO_MX7_Setup();          /* Initialize board IO */

    PORTSetPinsDigitalOut(IOPORT_B, SM_LEDS); /* Set PmodSTEP LEDs as output */
    PORTClearBits(IOPORT_B, SM_LEDS);         /* Set PmodSTEP LEDs off */

/* Enable multi-vector interrupts (Method suggested for XC32 */
    INTConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR);  /* Do only onec */
    INTEnableInterrupts();   /*Do as needed for global interrupt control */

} /* End of SetupHardware */

/* End of main.c */

