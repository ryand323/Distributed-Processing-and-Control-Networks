/* Project:     Project 2
 * File name:   P2.c
 * Author:      Ryan Donahue
 * Date:        9/14/2018
 * Description: 
******************************************************************************/
// Standard includes
#include <plib.h>

// Scheduler includes
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// Hardware settings
#include "chipKIT_Pro_MX7.h"

// Tasks
static void prvSetupHardware( void );
static void prvToggleLED( void *pvParameters );
static void prvLED_Handler( void *pvParameters);

// Idle Hook function
void vApplicationIdleHook();

// Assembly language wrapper
void __attribute__( (interrupt(ipl1), vector(_CHANGE_NOTICE_VECTOR))) vCN1_ISR_Wrapper( void );

// CN ISR
void vCN1_ISR_Handler( void );

// Semaphore for unblocking LED_Handler
xSemaphoreHandle xBiinarySemaphore;


/* main Function Description *************************************************
 * 
 * END DESCRIPTION **********************************************************/
int main( void )
{
    prvSetupHardware(); 
    vTraceEnable(TRC_START);
    vSemaphoreCreateBinary( xBinarySemaphore );
    xTaskCreate( prvToggleLED, "ToggleTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL );
    vTaskStartScheduler();
    return 0;
}

/* prvToggleLED Function Description *****************************************
 *
 * END DESCRIPTION **********************************************************/
static void prvToggleLED( void *pvParameters )
{
    for( ;; )
    {

    }
}

/* prvSetupHardware Function Description *************************************
 * 
 * END DESCRIPTION **********************************************************/
static void prvSetupHardware( void )
{
    chipKIT_PRO_MX7_Setup();
    
    // Setup IO
    PORTSetPinsDigitalOut(IOPORT_B, SM_LEDS);
    LATBCLR = SM_LEDS;
    
    // Enable multi-vector interrupts
    INTConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR);
    INTEnableInterrupts();
    portDISABLE_INTERRUPTS();
}

/* vApplicationStackOver Function Description ********************************
 * 
 * END DESCRIPTION **********************************************************/
void vApplicationStackOverflowHook( void )
{
	for( ;; );
}

/* _general_exception_handler Function Description ***************************
 * 
END DESCRIPTION *************************************************************/
void _general_exception_handler( unsigned long ulCause, unsigned long ulStatus )
{
    for( ;; );
}

