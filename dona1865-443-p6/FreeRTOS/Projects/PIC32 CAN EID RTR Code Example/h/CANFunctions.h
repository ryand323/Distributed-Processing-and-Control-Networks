/**********************************************************************
* © 2007 Microchip Technology Inc.
*
* FileName:        CANFunctions.h	 
* Dependencies:    Header (.h) files if applicable, see below
* Processor:       PIC32
* Compiler:        MPLAB® C32 
*
************************************************************************/

#ifndef _CAN_FUNCTIONS_H_
    #define _CAN_FUNCTIONS_H_

    #define CAN_BUS_SPEED 250000

/* This is the CAN1 FIFO message area.	 * Note the size of CAN1 message area.
 * It is 2 (Channels)*8 (Messages Buffers) 16 (bytes/per message buffer) bytes.
 */
    #define CAN1_CHANNELS       2
    #define CAN1_FIFO_BUFFERS   8
    #define CAN1_MSG_BUFF_SIZE  16
    #define CAN1_MSG_MEMORY  CAN1_CHANNELS*CAN1_FIFO_BUFFERS*CAN1_MSG_BUFF_SIZE
    BYTE CAN1MessageFifoArea[CAN1_MSG_MEMORY];

/* This is the CAN2 FIFO message area. Note: the size of CAN2 message area.
 * It is 1 (Channel) * 8 (Messages Buffers * 16 (bytes/per message buffer)
 * bytes.
 */
    #define CAN2_CHANNELS       1
    #define CAN2_FIFO_BUFFERS   8
    #define CAN2_MSG_BUFF_SIZE  16
    #define CAN2_MSG_MEMORY  CAN2_CHANNELS*CAN2_FIFO_BUFFERS*CAN2_MSG_BUFF_SIZE
    BYTE CAN2MessageFifoArea[CAN2_MSG_MEMORY];

    #define SID_BIT_MASK      0x07FF    /* Bit masking for generating SID */
    #define EID_BIT_MASK      0x03FFFF  /* Bit masking for generating EID */

    #define SID_FILTER_MASK   0x07FF    /* 11 bit mask - all bits must match */
    #define EID_FILTER_MASK   0x01FFFFFFF /* 29 bit mask - all bits must match*/

    #define LED1_INDICATION_MSG 0x008004001   /* 29 bit EID */

/* This message ID is not used in the RTR reference design */
    #define CAN2_MSG_ID         0x008004004   /* 29 bit EID */
#endif

/****************************************************************************
 * Function:    void CAN1Init(void);
 * 
 * Description:
 *   This function initializes CAN1 module. It sets up speed, FIFOs,
 *   filters and interrupts. FIFO0 is set up for TX with 8 message
 *   buffers. FIFO1 is set up for RX with 8 message buffers. Filter 0
 *   is set with Mask 0 for EID LED1_INDICATION_MSG. Only RXNEMPTY interrupt and
 * RBIF interrupt is enabled.
 * Precondition:    None.
 * Parameters:      None.
 * Return Values:   None.
 * Remarks:         None.
 * Example:     CAN1Init();
 * ***************************************************************************/
void CAN1Init(void);

/****************************************************************************
 * Function:    void CAN2Init(void);
 * Description:
 *   This function initializes CAN2 module. It sets up speed, FIFOs,
 *   filters and interrupts. FIFO0 is set up for TX with 8 message
 * 	 buffers with RTR enabled. Filter 0 is set with Mask 0 for EID
 * 	 LED1_INDICATION_MSG and points to FIFO0. Interrupts are not enabled.
 * Precondition:    None.
 * Parameters:      None.
 * Return Values:   None.
 * Remarks:         None.
 * Example:  CAN2Init();
  ***************************************************************************/
void CAN2Init(void);

/****************************************************************************
 * Function: void CAN1RxMsgProcess(void);
 * 
 * Description:
 *   This function checks if a message is available to be read in
 *   CAN1 FIFO1. If a message is available, the function will check
 *   byte 0 of the CAN message payload and will switch LED4 accordingly.
 * 
 * Precondition: 	None.
 * Parameters:   	None.
 * Return Values:	None.
 * Remarks:         None.
 * Example:  CAN1RxMsgProcess();
  ***************************************************************************/
void CAN1RxMsgProcess(void);

/****************************************************************************
 * Function:         void CAN2UpdateLEDMessage(BYTE led1Indication);
 * Description:
 *  This function checks if a CAN2 FIFO0 is empty. If empty it creates a
 *  message with EID LED1_INDICATION_MSG and single byte data payload
 *  consisting of led1Indication value which is use to control the on/off
 *  state of LED4. This message will be the response to RTR message sent
 * 	from CAN1 Tx.
 * 
 * Precondition:  None.
 * Parameters:    led1Indication - 1 to switch on LED4, 0 to switch it off.
 * Return Values: None.
 * Remarks:       None.
 * Example:  BYTE led4Command = 1;
 *           CAN2UpdateLEDMessage(led1Indication);
 ***************************************************************************/
 void CAN2UpdateLEDMessage(BYTE led1Indication);

/****************************************************************************
 * Function: void CAN1TxSendRTRMsg(void);
 *
 * Description:
 *   This function will send a RTR CAN message with EID LED1_INDICATION_MSG.
 *   Payload size of this message is zero. CAN2 will respond to this message
 *   with the status of LED1.
 *
 * Precondition:    None.
 * Parameters:      None.
 * Return Values:   None.
 * Remarks:         None.
 * Example:    CAN1TxSendRTRMsg();
 ***************************************************************************/
void CAN1TxSendRTRMsg(void);

/* End of CANFunctions.h*/
