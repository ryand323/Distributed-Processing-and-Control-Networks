/**********************************************************************
* © 2007 Microchip Technology Inc.
*
* FileName:        CANFunctions.h
* Dependencies:    Header (.h) files if applicable, see below
* Processor:       PIC32
* Compiler:        MPLAB® XC32
*
* Revisions:      6/24/13 RWW - TArget Cerebot MX7ck using XC32 1.30
*
*                 October 26, 2014 - Added peripheral library descriptions
*
************************************************************************/

#ifndef __CAN_FUNCTIONS_H__
#define __CAN_FUNCTIONS_H__

#include "plib.h"
//#include "GenericTypeDefs.h"

    #define CAN_BUS_SPEED 250000
//  #define CAN_BUS_SPEED 500000

/* This is the CAN1 FIFO message area.	*/
    #define CAN1_CHANNELS       3
    #define CAN1_FIFO_BUFFERS   8
    #define CAN1_MSG_BUFF_SIZE  16
    #define CAN1_MSG_MEMORY  CAN1_CHANNELS*CAN1_FIFO_BUFFERS*CAN1_MSG_BUFF_SIZE
    BYTE CAN1MessageFifoArea[CAN1_MSG_MEMORY];

/* This is the CAN2 FIFO message area.	*/
    #define CAN2_CHANNELS       2
    #define CAN2_FIFO_BUFFERS   8
    #define CAN2_MSG_BUFF_SIZE  16
    #define CAN2_MSG_MEMORY  CAN2_CHANNELS*CAN2_FIFO_BUFFERS*CAN2_MSG_BUFF_SIZE
    BYTE CAN2MessageFifoArea[CAN2_MSG_MEMORY];

    #define SID_MASK          0x07FF    /* Bit masking for generating SID */
    #define EID_MASK          0x03FFFF  /* Bit masking for generating EID */

    #define SID_FILTER_MASK   0x07FF    /* 11 bit mask - all bits must match */
    #define EID_FILTER_MASK   0x01FFFFFFF /* 29 bit mask - all bits must match*/

    #define CAN_EID_MSG_1     0x008004001  /* 29 bit EID */
    #define CAN_EID_MSG_2     0x008004002  /* 29 bit EID */
    #define CAN_EID_MSG_4     0x008004004  /* 29 bit EID */

/* This message ID is not used in the RTR reference design */
    #define CAN2_MSG_ID         0x008004004   /* 29 bit EID */
#endif

/****************************************************************************
 * Function: void CAN1Init(void);
 * Description:
 *      This function initializes CAN1 module. It sets up speed, FIFOs,
 *      filters and interrupts. 
 *      FIFO0 is set up for TX with 8 message buffers. 
 *      FIFO1 is set up for TX with 8 message buffers. 
 *      FIFO2 is set up for RX with 8 message buffers. 
 *      Filter 0 is set with mask 0 for EID 0x8004001. 
 *      Only RXNEMPTY interrupt and RBIF interrupt is enabled.
 *
 * Precondition:       None.
 * Parameters:         None.
 * Return Values:      None.
 * Remarks:            None.
 * Example:    CAN1Init();
  ***************************************************************************/
void CAN1Init(void);

/****************************************************************************
 * Function:          void CAN2Init(void);
 * Description:
 * 	    This function initializes CAN1 module. It sets up speed, FIFOs,
 * 	    filters and interrupts. FIFO0 is set up for TX with 8 message
 * 	    buffers. FIFO1 is set up for RX with 8 message buffers. Filter 0
 * 	    is set with Mask 0 for EID 0x8004002. Only RXNEMPTY interrupt and
 * 	    RBIF interrupt is enabled.
 *
 * Precondition:    None.
 * Parameters:      None.
 * Return Values:   None.
 * Remarks:         None.
 * Example:   CAN2Init();
  ***************************************************************************/
void CAN2Init(void);

/****************************************************************************
 * Function:            void CAN1RxMsgProcess(void);
 * Description:
 *    This function checks if a message is available to be read in
 *    CAN1 FIFO1. If a message is available, the function will check
 *    byte 0 (boolean flag) of the CAN message payload and will switch
 *    LED6 accordingly.
 * Precondition:    None.
 * Parameters:      None.
 * Return Values:   None.
 * Remarks:         None.
 *  Example:  CAN1RxMsgProcess();
  ***************************************************************************/
void CAN1RxMsgProcess(void);

/****************************************************************************
 * Function:            void CAN2RxMsgProcess(void);
 * Description:
 *   This function checks if a message is available to be read in
 *   CAN2 FIFO1. If a message is available, the function will check
 *   byte 0 (boolean flag) of the CAN message payload and will switch
 *   LED5 accordingly. It then calls CAN2TxSendLEDMsg().
 * 
 * Precondition:    None.
 * Parameters:      None.
 * Return Values:   None.
 * Remarks:         None.
 * Example:    CAN2RxMsgProcess();
  ***************************************************************************/
void CAN2RxMsgProcess(void);

/****************************************************************************
 * Function:    void CAN1TxSendLEDMsg(void);
 * Description:
 *     This function will send a CAN message with EID 0x8004002. In the payload,
 *     it sends one byte which is a toggle state for LED5. FIFO0 is used for
 *     sending the message.
 * Precondition:    None.
 * Parameters:      None.
 * Return Values:   None.
 * Remarks:         None.
 * Example:   CAN1TxSendLEDMsg();
  ***************************************************************************/
void CAN1TxSendRTRMsg(void);

/****************************************************************************
 * Function:    void CAN2TxSendLEDMsg(void);
 * Description:
 *    This function will send a CAN message with EID 0x8004001. In the payload,
 *    it sends one byte which is a toggle state for LED6. FIFO0 is used for
 *    sending the message.
 * Precondition:    None.
 * Parameters:      None.
 * Return Values:   None.
 * Remarks:         None.
 * Example:    CAN2TxSendLEDMsg();
  ***************************************************************************/
void CAN2TxSendIODataMsg(char *a, char *b, char *c, char *d, char *e, char *f, 
        char *g, char *h);

/* End of CANFunctions.h */
