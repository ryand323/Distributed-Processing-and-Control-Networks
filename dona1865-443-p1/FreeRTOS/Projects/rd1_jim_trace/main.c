/** @file main.c
 * 
 * @brief Change notice interrupt unblocks a handler task that operates LEDC in a push on/off manner.
 * Idle task indicates the state of BTN1 on LEDA.
 * Periodic task toggles LEDB every millisecond.
 * LEDD is active while the C-portion of the change notice ISR is active.
 * 
 * @par 
 */

// Standard includes.
#include <plib.h>

// Scheduler includes.
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

// Hardware settings.
#include "chipKIT_Pro_MX7.h"

// Set up the processor.
static void prvSetupHardware( void );

// Idle Hook function to monitor BTN1.
void vApplicationIdleHook();

// Handler and Periodic LED Toggle Tasks.
static void vHandlerTask( void *pvParameters );
static void vPeriodicTask( void *pvParameters );

// Function prototype for assembly language wrapper.
void __attribute__( (interrupt(ipl1), vector(_CHANGE_NOTICE_VECTOR))) vCN1_ISR_Wrapper( void );

// Semaphore Declaration.
xSemaphoreHandle xBinarySemaphore;

//C-portion of the CN ISR.
void CN_ISR_Handler( void );

// Global Variables for Toggling LEDA.
static int current_button = 0;
static int previous_button = 0;

// Global variable for counting LEDC Toggles.
static int LEDC_toggle_counter = 0;

int main( void )
{
    // Setup hardware.
    prvSetupHardware();
    // Initialize Tracealyzer recording.
    vTraceEnable(TRC_START);
    // Create handler task with high priority.
    xTaskCreate( vHandlerTask, ( signed char * ) "Handler", 240, NULL, 3, NULL );
    // Create periodic task to toggle LED B every ms.
    xTaskCreate( vPeriodicTask, ( signed char * ) "Periodic", 240, NULL, 1, NULL );
    // Create semaphore.
    vSemaphoreCreateBinary( xBinarySemaphore );
    // Start scheduler.
    vTaskStartScheduler();
    // Never reach here.
    for( ;; );
    return 0;
}

void vApplicationIdleHook( void )
{
    // LEDA enabled if BTN1 is pressed.
    current_button = PORTReadBits( IOPORT_G, BTN1 )>>6;
    // Invert LEDA if the current button status has changed.
    if( current_button != previous_button )
    {
        LATBINV = LEDA;
    }
    previous_button = current_button;
}

static void vHandlerTask( void *pvParameters )
{
    // Read the semaphore once. This ensures semaphore is empty before task
    // is run for the first time.
    xSemaphoreTake( xBinarySemaphore, 0);
    for( ;; )
    {
        // Block task until semaphore is received.
        xSemaphoreTake( xBinarySemaphore, portMAX_DELAY );
        // To get here, the event must have occurred. Process the event.
        // Debounce button.
        vTaskDelay( 20 / portTICK_RATE_MS );
        // If button is pressed, invert LED.
        if(PORTReadBits( IOPORT_G, BTN1 )>>6 == 1)
        {
        LATBINV = LEDC;
        LEDC_toggle_counter++;
        }
        // If button is still pressed, wait.
        while(PORTReadBits(IOPORT_G, BTN1) == 1);
        // Debounce again.
        vTaskDelay( 20 / portTICK_RATE_MS );
        // Enable CN interrupts again.
        mCNClearIntFlag();
        mCNIntEnable(1);
    }
}

static void vPeriodicTask( void *pvParameters )
{
    for( ;; )
    {
        // Toggle LEDB.
        LATBINV = LEDB;
        // 1 ms delay.
        vTaskDelay( 1 / portTICK_RATE_MS );
        // Stop Tracealyzer if LEDC toggles 4 times (LEDC lit twice).
//        if(LEDC_toggle_counter == 4)
//        {
//            for( ;; );
//        }
    }
}

void vCN1_ISR_Handler( void )
{
    // Assert LED D while in this ISR.
    LATBSET = LEDD;
    // Assign semaphore value.
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    // Give the semaphore to unblock the task.
    xSemaphoreGiveFromISR( xBinarySemaphore, &xHigherPriorityTaskWoken );
    portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
    // Extinguish LED D upon exiting ISR.
    LATBCLR = LEDD;
    // Prevent interrupts while servicing handler by disabling CN interrupts.
    mCNIntEnable(0);
}

static void prvSetupHardware( void )
{
    // Setup cerebot.
    chipKIT_PRO_MX7_Setup();
    // Setup outputs.
    PORTSetPinsDigitalOut(IOPORT_B, SM_LEDS);
    LATBCLR = SM_LEDS;
    // Enable Interrupts.
    INTConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR);
    INTEnableInterrupts();
    // Initialize change notice detection.
    cn_interrupt_initialize();
    portDISABLE_INTERRUPTS();
}

void cn_interrupt_initialize(void)
{
    // Make dummy variable
    unsigned int dummy; 
    // Enable CN for Buttons 1
    mCNOpen(CN_ON, CN8_ENABLE, 0);
    // Set priority levels group level 1, subgroup 0
    mCNSetIntPriority(1);
    mCNSetIntSubPriority(0);
    // Read port to clear difference
    dummy = PORTReadBits(IOPORT_G, BTN1);
    // Clear CN flag and enable interrupts
    mCNClearIntFlag();
    mCNIntEnable(1);
}

void vApplicationStackOverflowHook( void )
{
	for( ;; );
}

void _general_exception_handler( unsigned long ulCause, unsigned long ulStatus )
{
    for( ;; );
}