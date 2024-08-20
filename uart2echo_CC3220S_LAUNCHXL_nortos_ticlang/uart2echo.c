/*
 * Copyright (c) 2020, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== uart2echo.c ========
 */
#include <stdint.h>
#include <stddef.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART2.h>

/* Driver configuration */
#include "ti_drivers_config.h"

/*
 *  ======== mainThread ========
 */

/*States for the state machine */
enum LED_States {LED_Init, LED_START, LED_s0, LED_s1, LED_ON, LED_OFF} LED_State;


void *mainThread(void *arg0)
{
    char input;
    const char echoPrompt[] = "Echoing characters:\r\n";
    UART2_Handle uart;
    UART2_Params uartParams;
    size_t bytesRead;
    size_t bytesWritten = 0;
    uint32_t status     = UART2_STATUS_SUCCESS;

    /* Call driver init functions */
    GPIO_init();

    /* Configure the LED pin */
    GPIO_setConfig(CONFIG_GPIO_LED_0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);

    /* Create a UART where the default read and write mode is BLOCKING */
    UART2_Params_init(&uartParams);
    uartParams.baudRate = 115200;

    uart = UART2_open(CONFIG_UART2_0, &uartParams);

    if (uart == NULL)
    {
        /* UART2_open() failed */
        while (1) {}
    }

    /* Turn on user LED to indicate successful initialization */
    GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON);

    UART2_write(uart, echoPrompt, sizeof(echoPrompt), &bytesWritten);

    /* Loop forever echoing, State Machine loop */
    while (1)
        {
        switch (LED_State) {
        case LED_Init:
            LED_State = LED_START;
            break;

        case LED_START:
            bytesRead = 0;
            status = UART2_read(uart, &input, 1, &bytesRead);
            if (status == UART2_STATUS_SUCCESS && bytesRead > 0) {
                if (input == 'o') {
                    LED_State = LED_s0;
                } else {
                    LED_State = LED_START;
                }
            }
            break;

        case LED_s0:
            bytesRead = 0;
            status = UART2_read(uart, &input, 1, &bytesRead);
            if (status == UART2_STATUS_SUCCESS && bytesRead > 0) {
                if (input == 'n') {
                    LED_State = LED_ON;
                } else if (input == 'f') {
                    LED_State = LED_s1;
                } else {
                    LED_State = LED_START;
                }
            }
            break;

        case LED_s1:
            bytesRead = 0;
            status = UART2_read(uart, &input, 1, &bytesRead);
            if (status == UART2_STATUS_SUCCESS && bytesRead > 0) {
                if (input == 'f') {
                    LED_State = LED_OFF;
                } else {
                    LED_State = LED_START;
                }
            }
            break;


        /* Case used to turn on the LED light */
        case LED_ON:
            GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON);
            LED_State = LED_START;
            break;

        /* Case used to turn off the LED light */
        case LED_OFF:
            GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_OFF);
            LED_State = LED_START;
            break;

        default:
            LED_State = LED_Init;
            break;
        }
        }
}
