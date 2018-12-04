/**********************************************************************
 * FileName:        main.c
 * Dependencies:    Header (.h) files if applicable, see below
 * Processor:       PIC32MX7
 * Platform:        Digilent chipKIT Pro MX7
 * Compiler:        Microchip XC32 Ver. 1.30
 *
 * Author:          Richard Wall
 * Date:            October 28, 2014
 *
 *  ************************************************************************/

#include <plib.h>

#include "GenericTypeDefs.h"
#include "CANFunctions.h"
#include "chipKIT_Pro_MX7.h"

/* This code example runs on a chipKIT Pro MX7 board with a 8Mhz crystal.
 * The CAN1 and CAN2 connectors on the connected to each other to form a 2
 * node CAN bus.
 *
 * The code example shows how to use the  PIC32 CAN module to send and
 * receive Extended ID Remote Transmit Request (RTR) messages. In this
 * example, CAN1 sends an RTR request to CAN2 every 100ms for the next
 * status of LED1. CAN2 replies with a  message containing LED1 indication
 * (ON or OFF).  The code toggles the on/off state of LED1 once every second.
 * See documentation in CANFunctions.c for additional operations information. 
 * See Readme.pdf for a detailed description of the project functionality.
 *
 * Note: There is no CAN2 receive function needed for this application.
 * 
 */
 
#define ONE_SECOND        1000
#define TENTH_SECOND      100

/* Private function prototypes */
static void system_initialize(void);
static void processor_A(void);
static void processor_B(void);

/* Global variables */
static unsigned int tWait_sec;
static unsigned int tWait_100ms;

int main(void)
{
    system_initialize();    /* Resource initialization and configuration */

    while(1)    /* Continuously execute the code for processors A and B */
    {
        processor_A();
        processor_B();
    }
}

/****************************************************************************
 * Function: static void processor_A(void);
 *
 * Description:
 *  All CAN messages for processor A use the CAN1 module. This function sends
 *  for a request for remote message once each 100ms. The CAN Rx channel is
 *  polled as often as this function is called. All application control actions
 *  are implemented out of the "CAN1RxMsgProcess" function
 *
 *  Precondition:       System hardware must be initialized as well as the
 *                      interval timer "tWait_100ms".
 *  Parameters:         None.
 *  Return Values:      None.
 *  Remarks:       	"tWait_100ms" is a global variable modified by this
 *                      function.  This function implements a non-blocking time
 *                      delay.
 ***************************************************************************/
static void processor_A(void)
{
/* In this loop, we check if a 100 msec tick is available. If so then
 * CAN1 will send a RTR message to CAN2. Next we check if a 1 second
 * tick is available. If so then the state  of LED1 command is toggled.
 * CAN1 will  process received messages every time in the while(1) loop. */

    if(ReadCoreTimer() >= tWait_100ms) /* Is 100ms period expired? */
    {
/* If 100 milliseconds is up then CAN1 sends a RTR message to CAN2 to
 * control LED4. */
        CAN1TxSendRTRMsg();	/* Function is defined in CANFunctions.c */
        tWait_100ms = ReadCoreTimer()+(CORE_MS_TICK_RATE * TENTH_SECOND);
    }

/* CAN1RxMsgProcess() will check if CAN1 has received a message from CAN2 and
 * will switch LED4 on or off based on the LED command received from CAN2. */

    CAN1RxMsgProcess();
}

/****************************************************************************
 * Function: static void processor_B(void);
 *
 * Description:
 *  All CAN messages for processor B use the CAN2 module. This function toggles
 *  The led1Indication parameter once each second. After toggling the state of
 *  LED1 and the value of led1Indication parameter, the function
 *  "CAN2UpdateLEDMessage" is called which post but does not broadcast the
 *  updated message. The posting is not updated unless all previous postings
 *  have been read by another node by means of an RTR message requesting the
 *  posted message.
 *
 *  Precondition:       System hardware must be initialized as well as the
 *                      interval timer "tWait_sec".
 *  Parameters:         None.
 *  Return Values:      None.
 *  Remarks:       	"tWait_sec" is a global variable modified by this
 *                      function. This function implements a non-blocking time
 *                      delay.
 ***************************************************************************/
static void processor_B(void)
{
static BYTE led1Indication = 0;	/* 0 for LED4 OFF, 1 for LED4 ON. */

    if(ReadCoreTimer() >= tWait_sec) /* wait for 1 second */
    {
/* If 1 second is up then toggle the led LED1 and led1Indication. */
        LATGINV = LED1;         /* Use LED1 for local indication of the LED4 */
//      led1Indication = (LATG & LED1) && LED1; /* Read binary state of LED1 */
        led1Indication = (LATG & LED1) ? TRUE : FALSE;
/* CAN2UpdateLEDMessage will check if CAN2 is FIFO empty and populate
* it with a CAN message containing the latest state of LED4.*/
        CAN2UpdateLEDMessage(led1Indication);

/* Reset interval delay */
        tWait_sec = ReadCoreTimer() + (CORE_MS_TICK_RATE * ONE_SECOND);
    }
}

/****************************************************************************
 * Function: static void system_initialize(void);
 *
 * Description:         All hardware resources needed for control and
 *                      instrumentation are initialized as well as global
 *                      variables. 
 * 
 *  Precondition:       None.
 *  Parameters:         None.
 *  Return Values:      None.
 *  Remarks:       	Although both CAN1 and CAN2 are initialized,
 *                      if this reference design is implemented on two separate
 *                      processors, only one or the other would need to be
 *                      initialized.
 *
 *                      "tWait_sec" and tWait_100ms are global variables
 *                      used for non-blocking time delays are initialized by
 *                      this function.
 ***************************************************************************/
static void system_initialize(void)
{
/* Initialize the chipKIT Pro MX7 for maximum performance and configure the IO
 * pins for LED and BTN support. */
    chipKIT_PRO_MX7_Setup();   /* Function is defined in chipKIT_Pro_MX7.c */
    PORTSetPinsDigitalOut(IOPORT_B, SM_LEDS);
    PORTClearBits(IOPORT_B, SM_LEDS);

/* Configure system to enable multi vector interrupts. */
    INTConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR);
    INTEnableInterrupts();

 /* Functions are defined in CANFunctions.c 	*/
    CAN1Init();
    CAN2Init();

/* Used for non-blocking time delays */
    tWait_sec = ReadCoreTimer() + (CORE_MS_TICK_RATE * ONE_SECOND);
    tWait_100ms = ReadCoreTimer() + (CORE_MS_TICK_RATE * TENTH_SECOND);
}
/* End of main.c */
