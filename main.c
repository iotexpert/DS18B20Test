
#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "log.h"

#include "ntshell.h"
#include "ntlibc.h"
#include "psoc6_ntshell_port.h"

volatile int uxTopUsedPriority ;
TaskHandle_t blinkTaskHandle;

void blinkTask(void *arg)
{
    cyhal_gpio_init(CYBSP_USER_LED,CYHAL_GPIO_DIR_OUTPUT,CYHAL_GPIO_DRIVE_STRONG,0);

    for(;;)
    {
    	cyhal_gpio_toggle(CYBSP_USER_LED);
    	vTaskDelay(500);
    }
}


// Global variable with a handle to the shell
ntshell_t ntshell;

void ntShellTask()
{
    cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, CY_RETARGET_IO_BAUDRATE);

    printf("Started ntshell\n");
    setvbuf(stdin, NULL, _IONBF, 0);
    ntshell_init(
	       &ntshell,
	       ntshell_read,
	       ntshell_write,
	       ntshell_callback,
	       (void *)&ntshell);
    ntshell_set_prompt(&ntshell, "DS18B20> ");
    vtsend_erase_display(&ntshell.vtsend);
    ntshell_execute(&ntshell);
}


int main(void)
{
    cy_rslt_t result;
    uxTopUsedPriority = configMAX_PRIORITIES - 1 ; // enable OpenOCD Thread Debugging

    /* Initialize the device and board peripherals */
    result = cybsp_init() ;
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    __enable_irq();


    // Stack size in WORDs
    // Idle task = priority 0
    xTaskCreate(blinkTask, "blinkTask", configMINIMAL_STACK_SIZE,0 /* args */ ,2 /* priority */, &blinkTaskHandle);
    xTaskCreate(ntShellTask, "nt shell task", configMINIMAL_STACK_SIZE*3,0 /* args */ ,2 /* priority */, 0);

    vTaskStartScheduler();


}

/* [] END OF FILE */
