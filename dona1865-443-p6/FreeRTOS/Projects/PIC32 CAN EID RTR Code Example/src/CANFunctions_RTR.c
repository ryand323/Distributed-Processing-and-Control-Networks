/****************************************************************************
 * © 2009 Microchip Technology Inc.
 *
 * FileName:        CANFunctions.c
 * Dependencies:    Header (.h) files if applicable, see below
 * Processor:       PIC32
 * Compiler:        MPLAB® XC32
 *
 * Revised:         Richard W. Wall, 10/01/2013 - Target processor platform
 *                  is the Digilent Cerebot MX7
 *                  October 28, 2014 - Changed target processor platform to
 *                  the Digilent chipKIT Pro MX7 and added comments and timing
 *                  instrumentation.
 *
 * Description of operation:
 *
 * CAN1 module  uses FIFO0 for transmit and FIFO1  for receive. Each FIFO
 * will be configured to be 8 messages deep. CAN2 uses FIFO0 as transmit FIFO
 * with RTR enabled. FIFO size is set to 8. CAN module configuration code is
 * in CANFunctions.c

 * CAN1 module uses 2 Channels (Channel 0 and Channel 1). Each channel is
 * configured to be 8 messages deep. Channel 0 is configured for Transmit
 * and Channel 1 is configured for receive. CAN module configuration code
 * is in CANFunctions.c
 *
 * CAN2 module uses 1 channel (Channel 0). This channel is configured to be 8
 * messages deep. The channel is configured as a transmit channel and is RTR
 * enabled. When filter 0 receives RTR message, then Channel 0 will transmit
 * all it contents. There is no CAN2 receive function needed for this
 * application
 *
 * Note the size of CAN1 message area.
 * It is 2 (Channels)*8 (Messages Buffers) 16 (bytes/per message buffer) bytes.
 *
 * Note the size of CAN2 message area.
 * It is 1 (Channel) * 8 (Messages Buffers*16 (bytes/per message buffer) bytes.
 ****************************************************************************/

#include <plib.h>
#include "GenericTypeDefs.h"
#include "sw_timer.h"
#include "CANFunctions.h"
#include "chipKIT_Pro_MX7.h"

/* isCAN1MsgReceived is true if CAN1 FIFO1 received
 * a message. This flag is updated in the CAN1 ISR. */

/* isCAN1MsgReceived is true if CAN1 channel 1 received a message. This flag
 * is updated in the CAN1 ISR. */
static volatile BOOL isCAN1MsgReceived = FALSE;

/* isCAN2MsgReceived is true if CAN2 channel 1 received a message. This flag
 * is updated in the CAN2 ISR. */
static volatile BOOL isCAN2MsgReceived = FALSE;

/* Function Description ******************************************************
 * SYNTAX:          void CAN1Init(void);
 * KEYWORDS:        CAN1, initialize
 * DESCRIPTION:     This function initializes CAN1 for extended 29 bit ID. This
 *                  CAN1 controller is set up with two FIFO buffers that are 8
 *                  messages deep. Channel 0 is configured to be a Tx channel.
 *                  Channel 1 is configured as a receive channel and is set to
 *                  receive a specific message, in this case EID 0x8004001 by
 *                  the value specified for CAN_FILTER0.
 * PARAMETER1:      None
 * RETURN VALUE:    None
 * Notes:           None
 * END DESCRIPTION ************************************************************/
void CAN1Init(void)
{
    CAN_BIT_CONFIG canBitConfig;

/*  chipKIT Po MX7 +++++++++++++++++++++  */
    PORTSetPinsDigitalIn(IOPORT_F, BIT_12);	/* Set CAN1 Rx to input */
    PORTSetPinsDigitalOut(IOPORT_F, BIT_13);    /* Set CAN1 Tx  to output */
    ODCFSET    = BIT_13;       /* Make output pin open drain */
/* ++++++++++++++++++++++++++++++++++++++ */

/* The following is used to initialize CAN1 module. */

/* Step 1: Switch the CAN module ON and switch it to Configuration mode. */
    CANEnableModule(CAN1,TRUE);
    CANSetOperatingMode(CAN1, CAN_CONFIGURATION);/* Set CAN mode of operation */
    while(CANGetOperatingMode(CAN1) != CAN_CONFIGURATION);/*wait for operation*/

/* Step 2: Configure the Clock. The CAN_BIT_CONFIG data structure is used 
 * for this purpose. The propagation segment, phase segment 1 and phase
 * segment 2 are configured to have 3TQ. */
    canBitConfig.phaseSeg2Tq            = CAN_BIT_3TQ;
    canBitConfig.phaseSeg1Tq            = CAN_BIT_3TQ;
    canBitConfig.propagationSegTq       = CAN_BIT_3TQ;
    canBitConfig.phaseSeg2TimeSelect    = TRUE;
    canBitConfig.sample3Time            = TRUE;
    canBitConfig.syncJumpWidth          = CAN_BIT_2TQ;
    CANSetSpeed(CAN1,&canBitConfig, SYSTEM_FREQ, CAN_BUS_SPEED);
   
/* Step 3: Assign the buffer area to the CAN module. */
    CANAssignMemoryBuffer(CAN1,CAN1MessageFifoArea, CAN1_MSG_MEMORY);

/* Step 4: Configure channel 0 for TX and size of 8 message buffers with RTR 
 * disabled and low medium priority. Configure channel 1 for RX and size
 * of 8 message buffers and receive the full message. There can be from 1 to
 * 32 FIFO buffers each with up to 32 message buffers that are 16 bytes. Each
 * FIFO, if it is a receive buffer, has an ID filter that specifies which
 * message to accept. */
    CANConfigureChannelForTx(CAN1, CAN_CHANNEL0, CAN1_FIFO_BUFFERS,
                                   CAN_TX_RTR_DISABLED,
                                   CAN_LOW_MEDIUM_PRIORITY);

    CANConfigureChannelForRx(CAN1, CAN_CHANNEL1, CAN1_FIFO_BUFFERS,
                                    CAN_RX_FULL_RECEIVE);
	
/* Step 5: Configure filters and mask. Configure filter 0 to accept EID
 * messages with EID LED1_INDICATION_MSG. Configure Filter Mask is set to
 * compare all the ID bits and to filter by the ID type specified in the filter
 * configuration. Messages accepted by Filter 0 should be stored in Channel 1.
 */
	
    CANConfigureFilter      (CAN1, CAN_FILTER0, LED1_INDICATION_MSG, CAN_EID);

    CANConfigureFilterMask  (CAN1, CAN_FILTER_MASK0, EID_FILTER_MASK, CAN_EID,
                                CAN_FILTER_MASK_IDE_TYPE);

    CANLinkFilterToChannel  (CAN1, CAN_FILTER0, CAN_FILTER_MASK0, CAN_CHANNEL1);

    CANEnableFilter         (CAN1, CAN_FILTER0, TRUE);
	
/* Step 6: Enable interrupt and events. Enable the receive channel not empty
 * event (channel event) and the receive channel event (module event).
 * The interrupt peripheral library is used to enable the CAN interrupt to
 * the CPU. */

    CANEnableChannelEvent(CAN1, CAN_CHANNEL1, CAN_RX_CHANNEL_NOT_EMPTY, TRUE);
    CANEnableModuleEvent (CAN1, CAN_RX_EVENT, TRUE);

/* These functions are from interrupt peripheral library. */
    
    INTSetVectorPriority(INT_CAN_1_VECTOR, INT_PRIORITY_LEVEL_4);
    INTSetVectorSubPriority(INT_CAN_1_VECTOR, INT_SUB_PRIORITY_LEVEL_0);
    INTEnable(INT_CAN1, INT_ENABLED);

/* Step 7: Switch the CAN mode to normal mode. */

    CANSetOperatingMode(CAN1, CAN_NORMAL_OPERATION);
    while(CANGetOperatingMode(CAN1) != CAN_NORMAL_OPERATION);
}

/* Function Description ******************************************************
 * SYNTAX:          void CAN2Init(void);
 * KEYWORDS:        CAN2, initialize
 * DESCRIPTION:     This function initializes CAN1 for extended 29 bit ID. This
 *                  CAN2 controller is set up with two FIFO buffers that are 8
 *                  messages deep. Channel 0 is configured to be a Tx channel.
 * PARAMETER:       None
 * RETURN VALUE:    None
 * Notes:           None
 * END DESCRIPTION ************************************************************/
void CAN2Init(void)
{
CAN_BIT_CONFIG canBitConfig;

/* This function will initialize CAN2 module. */
/* chipKIT Pro MX7 ++++++++++++++++++++++*/
    PORTSetPinsDigitalIn(IOPORT_C, BIT_3);      /*Set CAN2 Rx */
    PORTSetPinsDigitalOut(IOPORT_C, BIT_2);     /* Set CAN2 Tx */
    ODCCSET    = BIT_2;     /* Set CAN2 Tx for open drain output */
/*++++++++++++++++++++++++++++++++++++++++*/

/* Step 1: Switch the CAN module ON and switch it to Configuration
 * mode. Wait till the switch is complete */
    CANEnableModule(CAN2,TRUE);

    CANSetOperatingMode(CAN2, CAN_CONFIGURATION);
    while(CANGetOperatingMode(CAN2) != CAN_CONFIGURATION);

/* Step 2: Configure the Clock. The CAN_BIT_CONFIG data structure is used
 * for this purpose. The propagation,  phase segment 1 and phase segment 2
 * are configured to have 3TQ. */
	
    canBitConfig.phaseSeg2Tq            = CAN_BIT_3TQ;
    canBitConfig.phaseSeg1Tq            = CAN_BIT_3TQ;
    canBitConfig.propagationSegTq       = CAN_BIT_3TQ;
    canBitConfig.phaseSeg2TimeSelect    = TRUE;
    canBitConfig.sample3Time            = TRUE;
    canBitConfig.syncJumpWidth          = CAN_BIT_2TQ;

    CANSetSpeed(CAN2, &canBitConfig, SYSTEM_FREQ, CAN_BUS_SPEED);
   
/* Step 3: Assign the buffer area to the CAN module. */ 

    CANAssignMemoryBuffer(CAN2,CAN2MessageFifoArea, CAN2_MSG_MEMORY);

/* Step 4: Configure channel 0 for TX and size of 8 message buffers with RTR
 * enable and low medium priority. */
    CANConfigureChannelForTx(CAN2,CAN_CHANNEL0,CAN2_FIFO_BUFFERS,
                                  CAN_TX_RTR_ENABLED,
                                  CAN_LOW_MEDIUM_PRIORITY);
	
/* Step 5: Configure filters and mask. Filter 0 is configured to accept a 
 * message with EID CAN2_MSG_ID. Note that filter 0 points to a
 * transmit channel (Channel 0). This is only possible because Channel 0 is
 * RTR enabled. */
	
    CANConfigureFilter      (CAN2, CAN_FILTER0, CAN2_MSG_ID, CAN_EID);
    CANConfigureFilterMask  (CAN2, CAN_FILTER_MASK0, EID_FILTER_MASK, CAN_EID,
                                CAN_FILTER_MASK_IDE_TYPE);
    CANLinkFilterToChannel  (CAN2, CAN_FILTER0,CAN_FILTER_MASK0,CAN_CHANNEL0); 
    CANEnableFilter         (CAN2, CAN_FILTER0, TRUE);							

/* Step 6: Enable Interrupts. */
/* CAN2 in this code example does not require any interrupts to be enabled. */

/*  INTSetVectorPriority(INT_CAN_2_VECTOR, INT_PRIORITY_LEVEL_4);
    INTSetVectorSubPriority(INT_CAN_2_VECTOR, INT_SUB_PRIORITY_LEVEL_0);
    INTEnable(INT_CAN2, INT_ENABLED);
*/
    
/* Step 7: Switch the CAN mode to normal mode. */
    CANSetOperatingMode(CAN2, CAN_NORMAL_OPERATION);
    while(CANGetOperatingMode(CAN2) != CAN_NORMAL_OPERATION);
}

/* Function Description ******************************************************
 * SYNTAX:          void CAN1RxMsgProcess(void);
 * KEYWORDS:        CAN1, Rx Message, process
 * DESCRIPTION:
 *      This function will check if a CAN  message is available. If so then it
 *      will read the Channel 1 . Byte 0 of the  incoming message will indicate
 *      if the LED4 should be switched ON or OFF.

 * PARAMETER:       None
 * RETURN VALUE:    None
 * Notes:           None
 * END DESCRIPTION ************************************************************/
void CAN1RxMsgProcess(void)
{
CANRxMessageBuffer * message;

    if(isCAN1MsgReceived == FALSE)
    {
/* CAN1 did not receive any message exit the function. Note that the
 * isCAN1MsgReceived flag is updated  by the CAN1 ISR. Toggle LEDD for event
 * timing instrumentation. */
        LATBINV = LEDD;
	return;
    }

/* CAN1 message received - toggle LEDE for event timing instrumentation. */
    LATBINV = LEDE;

/* Message was received. Reset this flag to catch the next message and read
 * the message. Note that you could also check the CANGetRxMessage function
 * return value for null to check if a message has  been received. */

    isCAN1MsgReceived = FALSE;
    message = (CANRxMessageBuffer *)CANGetRxMessage(CAN1,CAN_CHANNEL1);

/* Check the byte 0 of the data payload. If it is 0 then switch off LED4 else
 * switch it on. This is the remote indication. */
    if(message->data[0] == 0)
    {
        LATGCLR = LED4;
    }
    else
    {
        LATGSET = LED4;
    }

/* Call the CANUpdateChannel() function to let  the CAN module know that the
 * message processing is done. Enable the event so that the CAN module
 * generates an interrupt when the event occurs.*/

    CANUpdateChannel(CAN1, CAN_CHANNEL1);
    CANEnableChannelEvent(CAN1, CAN_CHANNEL1, CAN_RX_CHANNEL_NOT_EMPTY, TRUE);
}

/* Function Description ******************************************************
 * SYNTAX:          void CAN2UpdateLEDMessage(BYTE led4Command);
 * KEYWORDS:        CAN1, Rx Message, process
 * DESCRIPTION:
 *  This function will check if CAN2 Channel 0 is empty. The Channel becomes
 *  empty when it has replied to a RTR message from CAN1. If empty it will
 *  create a message with 1 byte data payload containing the value led4Conctol.
 *  LEDA is toggled each time the CAN message is updated. LEDB will toggle
 *  each time the function attempts to update but the TX buffer has not been
 *  polled for data.
 * 
 * PARAMETER:       None
 * RETURN VALUE:    None
 * Notes:           None
 * END DESCRIPTION ***********************************************************/
void CAN2UpdateLEDMessage(BYTE led1Indication)
{
CANTxMessageBuffer * message;
/* The CAN_TX_CHANNEL_EMPTY flag is set whenever any FIFO has a message that
 * has not been sent. */
    if((CANGetChannelEvent(CAN2,CAN_CHANNEL0) & CAN_TX_CHANNEL_EMPTY) == 0)
    {
/* CAN2 FIFO0 is not empty which means it did not receive any RTR message.
 * Toggle LEDB for event timing instrumentation. */
        PORTToggleBits(IOPORT_B, LEDB);
        return;
    }
	
/* CAN2 Channel 0 is empty. Create a message which CAN2 will auto transmit
 * when it receives a RTR message from CAN1. The message to be created will
 * will have EID LED1_INDICATION_MSG and will be accepted by Channel 1 on CAN1.
 * The data payload size is 1 byte and this represents the status of variable
 * led4Command.  LEDA is toggled to indicate that an updated CAN message is
 * ready to be requested remotely. */
	
/* Toggle LEDA for event timing instrumentation. */
    PORTToggleBits(IOPORT_B, LEDA);

/* Get a pointer to the next buffer in the channel. This pointer need not be
  * checked for null value because the channel is empty and will accommodate
  * a new message. */
    message = CANGetTxMessageBuffer(CAN2,CAN_CHANNEL0);

/* Form an extended ID CAN message. Start by clearing the message buffer. */
    message->messageWord[0] = 0;
    message->messageWord[1] = 0;
    message->messageWord[2] = 0;
    message->messageWord[3] = 0;

    message->msgSID.SID = (WORD) (LED1_INDICATION_MSG >> 18) & SID_BIT_MASK;
    message->msgEID.EID = (WORD) LED1_INDICATION_MSG & EID_BIT_MASK;
    message->msgEID.IDE = 1;		/* IDE = 1 means Extended ID message.*/
    message->msgEID.DLC = 1;		/* Send one byte of data.*/
    message->data[0] = led1Indication;	/* This is the payload.	*/

/* This function lets the CAN module know that the message processing is done
 * and message is ready to be processed. */
    CANUpdateChannel(CAN2,CAN_CHANNEL0);

/* Note that the CANFlushTxChannel() function is not called. Since the 
 * RTR is enabled for this channel, the CAN module will automatically
 * flush the channel when it receives an RTR message with EID 0x8004001. */
}

/* Function Description ******************************************************
 * SYNTAX:          void CAN1TxSendRTRMsg(void);
 * PARAMETER1       None
 * KEYWORDS:        RTR, CAN1, Tx
 * DESCRIPTION:     This function will send a RTR message to CAN2 with
 *                  EID CAN2_MSG_ID. This function is called every 10
 *                  millisecond. Both RTR and SRT bits are set to indicate this
 *                  is an RTR message. The DLC field of the RTR message is set
 *                  to 0 and hence the RTR message has no data payload.
 *                  LEDC is toggled each time a RTR message is sent.
 *
 * RETURN VALUE:    None
 * Notes:           None
 * END DESCRIPTION ************************************************************/
void CAN1TxSendRTRMsg(void)
{
CANTxMessageBuffer * message;

    PORTToggleBits(IOPORT_B, LEDC);

/* Get a pointer to the next buffer in the channel check if the returned
 * value is null. */
    message = CANGetTxMessageBuffer(CAN1,CAN_CHANNEL0);
    if(message!= NULL)
    {
/* Form a Standard ID RTR CAN message. Start by clearing the buffer. */
        message->messageWord[0] = 0;
        message->messageWord[1] = 0;
        message->messageWord[2] = 0;
        message->messageWord[3] = 0;

        message->msgSID.SID = (WORD) (CAN2_MSG_ID >> 18) & SID_BIT_MASK;
        message->msgEID.EID = (WORD) CAN2_MSG_ID & EID_BIT_MASK;
        message->msgEID.IDE = 1;    /* IDE = 1 means Extended ID message. */
        message->msgEID.RTR = 1;    /* For EID message Set RTR and SRR to 1 */
        message->msgEID.SRR = 1;    /* Set secondary RTR Bit */
        message->msgEID.DLC = 0;    /* No data payload for RTR messages. */

/* This function lets the CAN module know that the message processing is done
 * and message is ready to be processed. */

        CANUpdateChannel(CAN1,CAN_CHANNEL0);

/* Direct the CAN module to flush the TX channel. This will send any pending
 * message in the TX channel. */

        CANFlushTxChannel(CAN1,CAN_CHANNEL0);
    }
}

/* Function Description *******************************************************
 * SYNTAX:          CAN1InterruptHandler(void);
 * KEYWORDS:        RTR, CAN1, Tx
 * DESCRIPTION:     This is the CAN1 Interrupt Handler. Note that there are
 *                  many source events in the CAN1 module for this interrupt.
 *                  These events are enabled by the CANEnableModuleEvent()
 *                  function. In this example, only the RX_EVENT is enabled.
 *
 * PARAMETER:       None
 * RETURN VALUE:    None
 * Notes:           None
 * END DESCRIPTION ************************************************************/
void __ISR(_CAN_1_VECTOR, ipl4) CAN1InterruptHandler(void)
{

/* Check if the source of the interrupt is RX_EVENT. This is redundant since
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
 * event is persistent. You could either read the channel in the ISR to clear
 * the event condition or as done here, disable the event source, and set
 * an application flag to indicate that a message has been received. The event
 * can be enabled by the application when it has processed one message.
 *
 * Note that leaving the event enabled would cause the CPU to keep executing
 * the ISR since the CAN_RX_CHANNEL_NOT_EMPTY event is persistent (unless
 * the not empty condition is cleared.) */
			
            CANEnableChannelEvent(CAN1, CAN_CHANNEL1,
                                    CAN_RX_CHANNEL_NOT_EMPTY, FALSE);
            isCAN1MsgReceived = TRUE;
        }
    }

/* The CAN1 Interrupt flag is  cleared at the end of the interrupt routine. 
 * This is because the event source that could have caused this interrupt
 * to occur (CAN_RX_CHANNEL_NOT_EMPTY) is disabled. Attempting to clear the
 * CAN1 interrupt flag when the the CAN_RX_CHANNEL_NOT_EMPTY interrupt is
 * enabled will not have any effect because the base event is still present. */
	
    INTClearFlag(INT_CAN1);
}

/* Function Description *****************************************************
 * SYNTAX:          CAN2InterruptHandler(void);
 * KEYWORDS:        RTR, CAN2, Tx
 * DESCRIPTION:     This is the CAN2 Interrupt Handler. Note that there are
 *                  many source events in the CAN2 module for this interrupt.
 *                  These events are enabled by the CANEnableModuleEvent()
 *                  function. In this example, no EVENTs are enabled.
 *
 * PARAMETER:       None
 * RETURN VALUE:    None
 * Notes:           None
 * END DESCRIPTION ************************************************************/
void __ISR(_CAN_2_VECTOR, ipl4) CAN2InterruptHandler(void)
{
/* This is the CAN2 Interrupt Handler. Note that there are many events in the
 * CAN2 module that can cause this interrupt. These events are enabled by the
 * CANEnableModuleEvent() function. In this example, only the RX_EVENT is
 * enabled. */

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
 * event is persistent. You could either read the channel in the ISR to clear
 * the event condition or as done here, disable the event source, and set
 * an application flag to indicate that a message has been received. The event
 * can be enabled by the application when it has processed one message.
 *
 * Note that leaving the event enabled would cause the CPU to keep executing
 * the ISR since the CAN_RX_CHANNEL_NOT_EMPTY event is persistent (unless the
 * not empty condition is cleared.) */

            CANEnableChannelEvent(CAN2, CAN_CHANNEL1, CAN_RX_CHANNEL_NOT_EMPTY,
                                    FALSE);
            isCAN2MsgReceived = TRUE;
        }
    }

/* The CAN2 Interrupt flag is  cleared at the end of the interrupt routine.
 * This is because the event that could have caused this interrupt to occur
 * (CAN_RX_CHANNEL_NOT_EMPTY) is disabled. Attempting to clear the CAN2
 * interrupt flag when the the CAN_RX_CHANNEL_NOT_EMPTY interrupt is enabled
 * will not have any effect because the base event is still present. */

    INTClearFlag(INT_CAN2);
}// End of CAN2InterruptHandler(void)

/* End of CAHFunctions.c */