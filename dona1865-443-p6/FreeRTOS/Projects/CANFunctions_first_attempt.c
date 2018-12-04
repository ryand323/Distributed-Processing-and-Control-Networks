/**********************************************************************
* © 2007 Microchip Technology Inc.
*
* FileName:       CANFunctions.c
* Dependencies:   Header (.h) files if applicable, see below
* Processor:      PIC32
* Compiler:       Microchip XC32 ver. 1.30
* IDE:            MPLAB X (R) ver. 2.10
*
* Revisions:      6/24/13 RWW - TArget Cerebot MX7ck using XC32 1.30
*
*                 October 26, 2014 - Added peripheral library 
*                 descriptions - Target processor is the chipKIT Pro MX7
*
************************************************************************/
#include <plib.h>
#include "CANFunctions.h"
#include "chipKIT_Pro_MX7.h"
#include "GenericTypeDefs.h"

/* isCAN1MsgReceived is true if CAN1 FIFO1 received a message. This flag is
 * updated in the CAN1 ISR. */
static volatile BOOL isCAN1MsgReceived = FALSE;

/* isCAN2MsgReceived is true if CAN2 FIFO1 received
 * a message. This flag is updated in the CAN2 ISR. */
static volatile BOOL isCAN2MsgReceived = FALSE;

/* CAN1Init Function Description ********************************************
SYNTAX:         void CAN1Init(void);
KEYWORDS:       CAN1, initialize
DESCRIPTION:    This function initializes CAN1 for extended 29 bit ID
PARAMETER1:     None
RETURN VALUE:   None
Notes:          None
END DESCRIPTION ************************************************************/
void CAN1Init(void)
{
CAN_BIT_CONFIG canBitConfig;

/* chipKIT Pro MX7 +++++++++++++++++++++ */
    PORTSetPinsDigitalIn(IOPORT_F, BIT_12);	/* Set CAN1 Rx */
    PORTSetPinsDigitalOut(IOPORT_F, BIT_13);    /* Set CAN1 Tx */
    ODCFSET = BIT_13;        /* Set CAN Tx IO Pin for open drain */
/*++++++++++++++++++++++++++++++++++++++ */

/* This function will initialize CAN1 module. */
/* Step 1: Switch the CAN module ON and switch it to Configuration
 *  mode. Wait till the mode switch is complete. */

    CANEnableModule(CAN1,TRUE);

/* CAN set Operation Mode parameter description:
 * This routine sets the CAN operating mode. The CAN module requires itself to
 * be in certain modes in order to gain access to module functionality. Note
 * that after this function is called, it should be checked whether the mode
 * was set by using the CANGetOperatingMode() function.
 *
 * Parameter 1:     CAN Module:  CAN1 or CAN2
 * Parameter 2:     CAN Operation Mode - mutually exclusive:
 *                      CAN_NORMAL_OPERATION,
 *                      CAN_DISABLE
 *                      CAN_LOOPBACK
 *                      CAN_LISTEN_ONLY
 *                      CAN_CONFIGURATION
 *                      CAN_LISTEN_ALL_MESSAGES
*/
    CANSetOperatingMode(CAN1, CAN_CONFIGURATION);
    while(CANGetOperatingMode(CAN1) != CAN_CONFIGURATION);

 /* Step 2: Configure the Clock.The CAN_BIT_CONFIG data structure is used
  * for this purpose. The propagation segment, phase segment 1 and phase
  * segment 2 are configured to have 3TQ. CAN_BUS_SPEED is defined
  * in CANFunctions.h. GetSystemClock() is defined in chipKIT_Pro_MX7.h  */
	
    canBitConfig.phaseSeg2Tq            = CAN_BIT_3TQ;
    canBitConfig.phaseSeg1Tq            = CAN_BIT_3TQ;
    canBitConfig.propagationSegTq       = CAN_BIT_3TQ;
    canBitConfig.phaseSeg2TimeSelect    = TRUE;
    canBitConfig.sample3Time            = TRUE;
    canBitConfig.syncJumpWidth          = CAN_BIT_2TQ;

    CANSetSpeed(CAN1,&canBitConfig, GetSystemClock(), CAN_BUS_SPEED);

/* Step 3: Assign the buffer area to the CAN module. */

/* CAN Assign Channel Memory parameter description:
 * This routine assigns buffer memory to the CAN module. The CAN module uses
 * this buffer memory to store messages to be transmitted and received. The
 * size of the memory buffer should be enough to accommodate the required
 * number of message buffers and channels.
 *
 * Parameter 1:     CAN Module:  CAN1 or CAN2
 * Parameter 2:     CAN FIFO Memory - pointer to RAM allocated for channel msgs.
 * Parameter 4:     CAN FIFO Memory size - total number of bytes for RX and TX
*/
    CANAssignMemoryBuffer(CAN1,CAN1MessageFifoArea,(CAN1_MSG_MEMORY));

/* Step 4: 
 * Configure channel 0 for TX and size of 8 message buffers with RTR disabled 
 * and low medium priority.
 * Configure channel 1 for TX and size of 8 message buffers with RTR enabled 
 * and high medium priority.
 * Configure channel 2 for RX and size of 8 message buffers and receive the 
 * full message. 
*/

/* CAN Configure channel 0 and channel 1 for TX parameter description:
 * Parameter 1:     CAN Module:  CAN1 or CAN2
 * Parameter 2:     CAN FIFO Channel Number: CAN_CHANNEL0 to CAN_CHANNEL31
 * Parameter 3:     Numbers of FIFO buffers - 1 to 32
 * Parameter 4:     CAN RTR Request: CAN_TX_RTR_DISABLED / CAN_TX_RTR_ENABLED
 * Parameter 5:     CAN Sending priority:
 *                      CAN_LOWEST_PRIORITY
 *                      CAN_LOW_MEDIUM_PRIORITY
 *                      CAN_HIGH_MEDIUM_PRIORITY
 *                      CAN_HIGHEST_PRIORITY
*/
    CANConfigureChannelForTx(CAN1, CAN_CHANNEL0, CAN1_FIFO_BUFFERS,
                             CAN_TX_RTR_DISABLED,
                             CAN_LOW_MEDIUM_PRIORITY);
    
    CANConfigureChannelForTx(CAN1, CAN_CHANNEL1, CAN1_FIFO_BUFFERS,
                             CAN_TX_RTR_DISABLED,
                             CAN_LOW_MEDIUM_PRIORITY);

/* CAN Configure Channel 2 for RX parameter description:
 * Parameter 1:     CAN Module:  CAN1 or CAN2
 * Parameter 2:     CAN FIFO Channel Number: CAN_CHANNEL0 to CAN_CHANNEL31
 * Parameter 3:     Numbers of FIFO buffers - 1 to 32
 * Parameter 4:     CAN RTR RX mode: CAN_RX_DATA_ONLY or CAN_RX_FULL_RECEIVE
*/
    CANConfigureChannelForRx(CAN1, CAN_CHANNEL2, CAN2_FIFO_BUFFERS,
                             CAN_RX_DATA_ONLY);
	
/* Step 5: Configure filters and mask. Configure filter 0 to accept EID 
 * messages with ID 0x8004001. Configure filter mask 0 to compare all the ID
 * bits and to filter by the ID type specified in the filter configuration.
 * Messages accepted by filter 0 should be stored in channel 2. */

/* CAN Configure Filter parameter description:
 * Parameter 1:     CAN Module:  CAN1 or CAN2
 * Parameter 2:     CAN Filter number: CAN_FILTER0 - CAN_FILTER31
 * Parameter 3:     CAN ID: 0x000000000 to 0x01FFFFFFF
 * Parameter 4:     CAN ID Type: CAN_EID or CAN_SID
 */
    CANConfigureFilter(CAN1, CAN_FILTER0, CAN_EID_MSG_1, CAN_EID);

/* CAN Configure Filter Mask parameter description:
 * Parameter 1:     CAN Module:  CAN1 or CAN2
 * Parameter 2:     CAN Filter Mask: CAN_FILTER_MASK0 - CAN_FILTER_MASK3
 * Parameter 3:     CAN Mask:   0x0 - 0x01FFFFFFF
 * Parameter 4:     CAN ID Type: CAN_EID or CAN_SID
 * Parameter 5:     CAN Filter Mask Type: CAN_FILTER_MASK_IDE_TYPE or
 *                                        CAN_FILTER_MASK_ANY_TYPE
 */
    CANConfigureFilterMask(CAN1, CAN_FILTER_MASK0, EID_FILTER_MASK,
                              CAN_EID, CAN_FILTER_MASK_IDE_TYPE);

/* CAN Link Channel to Filter to Mask parameter description:
 * Parameter 1:     CAN Module:  CAN1 or CAN2
 * Parameter 2:     CAN Filter number: CAN_FILTER0 to CAN_FILTER31
 * Parameter 3:     CAN Mask number: CAN_FILTER_MASK0 to CAN_FILTER_MASK3
 * Parameter 4:     CAN FIFO Channel Number: CAN_CHANNEL0 to CAN_CHANNEL31
 */
    CANLinkFilterToChannel(CAN1, CAN_FILTER0, CAN_FILTER_MASK0, CAN_CHANNEL2);

/* CAN Filter Enable parameter description:
 * Parameter 1:     CAN Module:  CAN1 or CAN2
 * Parameter 2:     CAN Filter number: CAN_FILTER0 to CAN_FILTER31
 * Parameter 3:     ENABLE: TURE or FALSE
 */
    CANEnableFilter(CAN1, CAN_FILTER0, TRUE);
	
/* Step 6: Enable interrupt and events. Enable the receive channel not empty
 * event (channel event) and the receive channel event (module event).
 * The interrupt peripheral library is used to enable the CAN interrupt to
 * the CPU. */

/* CAN Enable Channel Event parameter description:
 * This routine enables or disables channel level events. Any enabled channel
 * event will cause a CAN module event. An event can be active regardless of
 * it being enabled or disabled. Enabling a TX type of event for a RX channel
 * will have no effect.
 * 
 * Parameter 1:     CAN Module:  CAN1 or CAN2
 * Parameter 2:     CAN FIFO Channel Number: CAN_CHANNEL0 to CAN_CHANNEL31
 * Parameter 3:     CAN Channel event type - any combination:   	
 *                      CAN_RX_CHANNEL_NOT_EMPTY
 *                      CAN_RX_CHANNEL_HALF_FULL
 *                      CAN_RX_CHANNEL_FULL
 *                      CAN_RX_CHANNEL_OVERFLOW
 *                      CAN_RX_CHANNEL_ANY_EVENT
 *                      CAN_TX_CHANNEL_EMPTY
 *                      CAN_TX_CHANNEL_HALF_EMPTY
 *                      CAN_TX_CHANNEL_NOT_FULL
 *                      CAN_TX_CHANNEL_ANY_EVENT
 *  Parameter 4:    Event enable - TRUE or FALSE 
 */
    CANEnableChannelEvent(CAN1, CAN_CHANNEL2, CAN_RX_CHANNEL_NOT_EMPTY, TRUE);

/* CAN Enable Module Event parameter description:
 * This routine enables or disables module level events. Any enabled module
 * event will cause the CAN module to generate a CPU interrupt. An event can
 * be active regardless of it being enabled or disabled.
 *
 * Parameter 1:     CAN Module:  CAN1 or CAN2
 * Parameter 2:     CAN Event flags - any combination:
 *                      CAN_TX_EVENT
 *                      CAN_RX_EVENT
 *                      CAN_TIMESTAMP_TIMER_OVERFLOW_EVENT
 *                      CAN_OPERATION_MODE_CHANGE_EVENT
 *                      CAN_RX_OVERFLOW_EVENT
 *                      CAN_SYSTEM_ERROR_EVENT
 *                      CAN_BUS_ERROR_EVENT
 *                      CAN_BUS_ACTIVITY_WAKEUP_EVENT
 *                      CAN_INVALID_RX_MESSAGE_EVENT
 * Parameter 3:     Enable CAN events: TRUE or FALSE
*/
    CANEnableModuleEvent (CAN1, CAN_RX_EVENT, TRUE);

/* These functions are from interrupt peripheral library. */
    INTSetVectorPriority(INT_CAN_1_VECTOR, INT_PRIORITY_LEVEL_4);
    INTSetVectorSubPriority(INT_CAN_1_VECTOR, INT_SUB_PRIORITY_LEVEL_0);
    INTEnable(INT_CAN1, INT_ENABLED);

/* Step 7: Switch the CAN mode to normal mode. */
    CANSetOperatingMode(CAN1, CAN_NORMAL_OPERATION);
    while(CANGetOperatingMode(CAN1) != CAN_NORMAL_OPERATION);

} /* End of CAN1Init */

/* CAN2Init Function Description ********************************************
SYNTAX:         void CAN2Init(void);
KEYWORDS:       CAN2, initialize
DESCRIPTION:    This function initializes CAN2 for extended 29 bit ID
PARAMETER1:     None
RETURN VALUE:   None
Notes:          See CAN1Init for function descriptions and program details
END DESCRIPTION ************************************************************/
void CAN2Init(void)
{
    CAN_BIT_CONFIG canBitConfig;

/* chipKIT Pro MX7 +++++++++++++++++++++ */
    PORTSetPinsDigitalIn(IOPORT_C, BIT_3);  /* Set CAN2 Rx */
    PORTSetPinsDigitalOut(IOPORT_C, BIT_2); /* Set CAN2 Tx */
    ODCCSET = BIT_2;         /* Set CAN Tx IO pin for open drain */
/* +++++++++++++++++++++++++++++++++++++ */

 /* Step 1: Enable CAN Module and set into configuration mode. */
    CANEnableModule(CAN2,TRUE);

    CANSetOperatingMode(CAN2, CAN_CONFIGURATION);
    while(CANGetOperatingMode(CAN2) != CAN_CONFIGURATION);

 /* Step 2: Configure the Clock. */
	
    canBitConfig.phaseSeg2Tq            = CAN_BIT_3TQ;
    canBitConfig.phaseSeg1Tq            = CAN_BIT_3TQ;
    canBitConfig.propagationSegTq       = CAN_BIT_3TQ;
    canBitConfig.phaseSeg2TimeSelect    = TRUE;
    canBitConfig.sample3Time            = TRUE;
    canBitConfig.syncJumpWidth          = CAN_BIT_2TQ;

    CANSetSpeed(CAN2, &canBitConfig, GetSystemClock(), CAN_BUS_SPEED);

/* Step 3: Assign the buffer area to the CAN module. */
    CANAssignMemoryBuffer(CAN2,CAN2MessageFifoArea,CAN2_MSG_MEMORY);

/* Step 4: Configure channel 0  */
    CANConfigureChannelForTx(CAN2,CAN_CHANNEL0,CAN2_FIFO_BUFFERS,
                             CAN_TX_RTR_ENABLED,
                             CAN_LOW_MEDIUM_PRIORITY);

    CANConfigureChannelForRx(CAN2,CAN_CHANNEL1,CAN2_FIFO_BUFFERS,
                             CAN_RX_FULL_RECEIVE);
	
 /* Step 5: Configure filters and mask. */
    //CHANNEL 0
    CANConfigureFilter(CAN2, CAN_FILTER1, CAN_EID_MSG_2 , CAN_EID);
    CANConfigureFilterMask(CAN2, CAN_FILTER_MASK1, EID_FILTER_MASK,
                                CAN_EID, CAN_FILTER_MASK_IDE_TYPE);
    CANLinkFilterToChannel(CAN2, CAN_FILTER1, CAN_FILTER_MASK1, CAN_CHANNEL0);
    CANEnableFilter(CAN2, CAN_FILTER1, TRUE);
	
    //CHANNEL 1
    CANConfigureFilter(CAN2, CAN_FILTER2, CAN_EID_MSG_4 , CAN_EID);
    CANConfigureFilterMask(CAN2, CAN_FILTER_MASK2, EID_FILTER_MASK,
                                CAN_EID, CAN_FILTER_MASK_IDE_TYPE);
    CANLinkFilterToChannel(CAN2, CAN_FILTER2, CAN_FILTER_MASK2, CAN_CHANNEL1);
    CANEnableFilter(CAN2, CAN_FILTER2, TRUE);

    
/* Step 6: Enable interrupt and events.  */
    CANEnableChannelEvent(CAN2, CAN_CHANNEL1, CAN_RX_CHANNEL_NOT_EMPTY, TRUE);
    CANEnableModuleEvent(CAN2, CAN_RX_EVENT, TRUE);

 /* These functions are from interrupt peripheral library. */
    INTSetVectorPriority(INT_CAN_2_VECTOR, INT_PRIORITY_LEVEL_4);
    INTSetVectorSubPriority(INT_CAN_2_VECTOR, INT_SUB_PRIORITY_LEVEL_0);
    INTEnable(INT_CAN2, INT_ENABLED);
    
 /* Step 7: Switch the CAN mode to normal mode. */
    CANSetOperatingMode(CAN2, CAN_NORMAL_OPERATION);
    while(CANGetOperatingMode(CAN2) != CAN_NORMAL_OPERATION);
} /* End of CAN2Init */

/****************************************************************************
 * Function:    void CAN1RxMsgProcess(void);
 * Description:
 *   This function checks if a message is available to be read in
 *   CAN1 FIFO1. If a message is available, the function will check
 *   byte 0 (boolean flag) of the CAN message payload and will switch
 *   LED3 and LEDC accordingly.
 * Precondition: 	None.
 * Parameters:   	None.
 * Return Values:	None.
 * Remarks:   	None.
 * Example:        CAN1RxMsgProcess();
  ***************************************************************************/
void CAN1RxMsgProcess(void)
{
/* LEDCStatus tracks the status of LEDC.*/
static BYTE	LEDCStatus = 0;
 /* This function will check if a CAN message is available in CAN1 channel 1.
  * If so , the message is read. Byte 0 of the received message will indicate
  * if LEDC and LED3 should be switched ON or OFF. */

    CANRxMessageBuffer * message;

    if(isCAN1MsgReceived == FALSE)
    {
/* CAN1 did not receive any message so exit the function. Note that the
 * isCAN1MsgReceived flag is updated by the CAN1 ISR. */
        return;
    }
	
/* Message was received. Reset message received flag to catch the next message 
 * and read the message. Note that you could also check the CANGetRxMessage
 * function return value for null to check if a message has been received. */
    isCAN1MsgReceived = FALSE;	
    message = (CANRxMessageBuffer *)CANGetRxMessage(CAN1,CAN_CHANNEL1);

/* Check byte 0 of the data payload. If it is 0 then switch off LED6 else
 * switch it on. */
    if(message->data[0] == 0)
    {
        PORTClearBits(IOPORT_G, LED3);
        PORTClearBits(IOPORT_B, LEDC);
    }
    else
    {
        PORTSetBits(IOPORT_G, LED3);
        PORTSetBits(IOPORT_B, LEDC);
    }

/* Call the CANUpdateChannel() function to let CAN 1 module know that the
 * message processing is done. Enable the receive channel not empty event
 * so that the CAN module generates an interrupt when the event occurs the
 * next time.*/
    CANUpdateChannel(CAN1, CAN_CHANNEL1);
    CANEnableChannelEvent(CAN1, CAN_CHANNEL1, CAN_RX_CHANNEL_NOT_EMPTY, TRUE);
} /* End of CAN1RxMsgProcess */

/****************************************************************************
 * Function:    void CAN2RxMsgProcess(void);
 * Description:
 *   This function checks if a message is available to be read in
 *   CAN2 FIFO1. If a message is available, the function will check
 *   byte 0 (boolean flag) of the CAN message payload and will switch
 *   LEDD and LED4 accordingly. It then calls CAN2TxSendLEDMsg().
 * Precondition:    None.
 * Parameters:      None.
 * Return Values:   None.
 * Remarks:         None.
 * Example:    CAN2RxMsgProcess();
  ***************************************************************************/
void CAN2RxMsgProcess(void)
{
/* This function will check if CAN 2 received a message from CAN1.
 * If so it will then read CAN2 Channel 1. Byte 0 of the received message
 * will indicate if the LED4 and LEDD should be switched ON or OFF. */

    CANRxMessageBuffer * message;   /* Pointer to the CAN message buffer */

    if(isCAN2MsgReceived == FALSE)
    {
/* CAN2 did not receive any message so exit the function. Note that the
 * isCAN2MsgReceived flag is updated by the CAN2 ISR. */
        return;
    }
	
/* Message was received. Reset isCAN2MsgReceived flag to catch the next
 * message. */
    isCAN2MsgReceived = FALSE;
    message = (CANRxMessageBuffer *)CANGetRxMessage(CAN2,CAN_CHANNEL1);

/* Check byte 0 of the data payload. If it is 0 then switch off LED3 else
 * switch it on. */
    if(message->data[0] == 0)
    {
        PORTClearBits(IOPORT_G, LED4);
        PORTClearBits(IOPORT_B, LEDD);
    }
    else
    {
        PORTSetBits(IOPORT_G, LED4);
        PORTSetBits(IOPORT_B, LEDD);
    }

/* Call the CANUpdateChannel() function to let the CAN module know that the
 * message processing is done. Enable the event so that the CAN module
 * generates an interrupt when the event occurs.*/
    CANUpdateChannel(CAN2, CAN_CHANNEL1);
    CANEnableChannelEvent(CAN2, CAN_CHANNEL1, CAN_RX_CHANNEL_NOT_EMPTY, TRUE);

/* Send a message to CAN1 to toggle LEDB. */
    CAN2TxSendRTRMsg();
} /* End of CAN2RxMsgProcess*/

/****************************************************************************
 * Function:    void CAN1TxSendRTRMsg(void);
 * Description:
 *   This function will send a message to CAN2 with EID 0x8004001. The data 
 *   payload size is 1 byte. This is an RTR request.value of the LEDAStatus will be toggled and
 *   then sent as the payload. CAN1 Channel 0 is used to send the message.
 * Precondition:    None.
 * Parameters:      None.
 * Return Values:   None.
 * Remarks:         None.
 * Example:  CAN2TxSendRTRMsg();
  ***************************************************************************/
void CAN1TxSendRTRMsg(void)
{
CANTxMessageBuffer * message; /* Pointer to the CAN message buffer */

/* Get a pointer to the next buffer in the channel check if the returned
 * value is null. */
    message = CANGetTxMessageBuffer(CAN1,CAN_CHANNEL0);
    if(message != NULL)
    {
 /* Form a Extended ID CAN message. Start by clearing the buffer. */
        message->messageWord[0] = 0;
        message->messageWord[1] = 0;
        message->messageWord[2] = 0;
        message->messageWord[3] = 0;

        message->msgSID.SID = (WORD) (CAN_EID_MSG_2 >> 18) & SID_MASK;
        message->msgEID.EID = (WORD) CAN_EID_MSG_2  & EID_MASK;
        message->msgEID.IDE	= 1;        // IDE = 1 means EID message.
        message->msgEID.RTR	= 1;        // RTR message.
        message->msgEID.SRR = 1;        // Set secondary RTR Bit.
        message->msgEID.DLC	= 0;        // Send no data.

/* This function lets the CAN module know that the message processing is done
 * and message is ready to be processed. */
        CANUpdateChannel(CAN1,CAN_CHANNEL0);

/* Direct the CAN module to flush the TX channel. This will send any pending
 * message in the TX channel. */
        CANFlushTxChannel(CAN1,CAN_CHANNEL0);
    }
} /* End of CAN1TxSendLEDMsg */

/****************************************************************************
 * Function:    void CAN2TxSendIODataMsg(void);
 * Description:
 *   This function will send a CAN message with EID 0x8004001. In the payload,
 *   it sends eight byte which is PWM setting, Temp measurement, and RPS.
 *   FIFO1 sends the message.
 * Precondition:    None.
 * Parameters:      None.
 * Return Values:   None.
 * Remarks:         None.
 * Example:    CAN2TxSendIODataMsg();
  ***************************************************************************/
void CAN2TxSendIODataMsg(char *a, char *b, char *c, char *d, char *e, char *f, 
        char *g, char *h)
{
    CANTxMessageBuffer * message;		/* Pointer to the CAN message buffer */

/* Get a pointer to the next buffer in the channel check if the returned value
 * is null. */
    message = CANGetTxMessageBuffer(CAN2,CAN_CHANNEL0);
    if(message != NULL)
    {
/* Form a Extended ID message. Start by clearing the buffer. */
        message->messageWord[0] = 0;
        message->messageWord[1] = 0;
        message->messageWord[2] = 0;
        message->messageWord[3] = 0;

        message->msgSID.SID = (WORD) (CAN_EID_MSG_1 >> 18) & SID_MASK;
        message->msgEID.EID = (WORD) CAN_EID_MSG_1  & EID_MASK;
        message->msgEID.IDE = 1;            /* IDE = 1 means EID message. */
        message->msgEID.RTR = 0;            /* Not an RTR message. */
        message->msgEID.DLC = 8;            /* Send eight bytes of data. */
        message->data[0]    = *a;
        message->data[1]    = *b;
        message->data[2]    = *c;
        message->data[3]    = *d;
        message->data[4]    = *e;
        message->data[5]    = *f;
        message->data[6]    = *g;
        message->data[7]    = *h;

/* This function lets the CAN module know that the message processing is done
 * and message is ready to be processed. */
        CANUpdateChannel(CAN2,CAN_CHANNEL0);

 /* Direct the CAN module to flush the TX channel. This will send any pending
  * message in the TX channel. */
        CANFlushTxChannel(CAN2,CAN_CHANNEL0);
    }
}

/****************************************************************************
 * Function:    void __ISR(_CAN_1_VECTOR, ipl4) CAN1InterruptHandler(void);
 * Description:
 *  This is the CAN1 Interrupt Handler. Note that there are many source events
 *  in the CAN1 module for this interrupt. These events are enabled by the
 *  CANEnableModuleEvent() function. In this example, only the RX_EVENT
 *  is enabled.
 * Precondition:    None.
 * Parameters:      None.
 * Return Values:   None.
 * Remarks:         ISR cannot be called directly.
  ***************************************************************************/
void __ISR(_CAN_1_VECTOR, ipl4) CAN1InterruptHandler(void)
{

/* Check if the source of the interrupt is  RX_EVENT. This is redundant since
 * only this event is enabled in this example but this shows one scheme for
 * handling events. */

    if((CANGetModuleEvent(CAN1) & CAN_RX_EVENT) != 0)
    {
/* Within this, you can check which channel caused the event by using the 
 * CANGetModuleEvent() function which returns a code representing the highest
 * priority pending event. */
        if(CANGetPendingEventCode(CAN1) == CAN_CHANNEL1_EVENT)
        {
/* This means that channel 1 caused the event. The CAN_RX_CHANNEL_NOT_EMPTY
 * event is persistent. You could either read the channel in the ISR
 * to clear the event condition or as done here, disable the event source,
 * and set an application flag to indicate that a message  has been received.
 * The event can be enabled by the application when it has processed one
 * message. */

/* Note that leaving the event enabled would cause the CPU to keep executing
 * the ISR since the CAN_RX_CHANNEL_NOT_EMPTY event is persistent (unless
 * the not empty condition is cleared.)  */
            CANEnableChannelEvent(CAN1, CAN_CHANNEL1, CAN_RX_CHANNEL_NOT_EMPTY,
                                    FALSE);

            isCAN1MsgReceived = TRUE;    /* CAN Message received flag */
        }
    }

/* The CAN1 Interrupt flag is cleared at the end of the interrupt routine. 
 * This is because the interrupt source that could have caused this interrupt
 * to occur (CAN_RX_CHANNEL_NOT_EMPTY) is disabled. Attempting to clear the
 * CAN1 interrupt flag when the the CAN_RX_CHANNEL_NOT_EMPTY interrupt is
 * enabled will not have any effect because the base event is still present. */	
    INTClearFlag(INT_CAN1);
}
/****************************************************************************
 * Function:    void __ISR(_CAN_2_VECTOR, ipl4) CAN2InterruptHandler(void);
 * Description:
 * This is the CAN2 Interrupt Handler. Note that there are many events in the
 * CAN2 module that can cause this interrupt. These events are enabled by the
 * CANEnableModuleEvent() function. In this example, only the RX_EVENT is
 * enabled.
 * Precondition:    None.
 * Parameters:      None.
 * Return Values:   None.
 * Remarks:         ISR cannot be called directly
  ***************************************************************************/
void __ISR(_CAN_2_VECTOR, ipl4) CAN2InterruptHandler(void)
{  
/* Check if the source of the interrupt is RX_EVENT. This is redundant since 
 * only this event is enabled in this example but this shows one scheme for
 * handling interrupts. */

    if((CANGetModuleEvent(CAN2) & CAN_RX_EVENT) != 0)
    {
/* Within this, you can check which event caused the interrupt by using the 
 * CANGetPendingEventCode() function to get a code representing the highest
 * priority active event.*/
		
        if(CANGetPendingEventCode(CAN2) == CAN_CHANNEL1_EVENT)
        {
/* This means that channel 1 caused the event. The CAN_RX_CHANNEL_NOT_EMPTY
 * event is persistent. You could either read the channel in the ISR
 * to clear the event condition or as done here, disable the event source,
 * and set  an application flag to indicate that a message has been received.
 * The event can be enabled by the application when it has processed one
 * message.
 *
 * Note that leaving the event enabled would cause the CPU to keep executing the
 * ISR since the CAN_RX_CHANNEL_NOT_EMPTY event is persistent (unless the not
 * empty condition is cleared.)
 */			
        CANEnableChannelEvent(CAN2, CAN_CHANNEL1, CAN_RX_CHANNEL_NOT_EMPTY, 
                                FALSE);

        isCAN2MsgReceived = TRUE;    /* CAN Message received flag */
        }
    }

/* The CAN2 Interrupt flag is  cleared at the end of the interrupt routine. 
 * This is because the event that could have caused this interrupt  to occur
 * (CAN_RX_CHANNEL_NOT_EMPTY) is disabled. Attempting to clear the CAN2
 * interrupt flag when the the CAN_RX_CHANNEL_NOT_EMPTY interrupt is enabled
 * will not have any effect because the base event is still present. */
	
    INTClearFlag(INT_CAN2);
} /* enD OF CAN2InterruptHandler(void) */

/* End of CANFunctions.c */
