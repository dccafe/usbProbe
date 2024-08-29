#include <msp430.h> // WDTCTL
#include <stdlib.h> // uintXX_t
#include <ctype.h>  // atoi
#include <string.h> // strtok
#include <stdio.h>  // sprintf

// P1.2, P1.3, P1.4,  P1.5
// S1,   S2,   GREEN, RED

#include "usb/usb_utils.h"
#include "modules/gpio/gpio.h"
#include "modules/timer/timer.h"
#include "modules/serial/serial.h"
#include "modules/adc/adc.h"

#define MAX_BUFFER_SIZE 256

uint8_t echo = 0;

void main (void)
{
    WDTCTL = WDTPW | WDTHOLD;

    P1DIR = 0;
    usb_init();
    timer_init();

    while (1)
    {
        char container[MAX_BUFFER_SIZE];
        usb_read_until(container, '\n');

        char * module = strtok(container, " "); // gpio, timer, serial or adc
        char * action = strtok(0, " ");         // read or write
        char * Pxy    = strtok(0, " ");         // Px.y
        uint8_t port = *(Pxy+1) - '0';          //  x |
        uint8_t pin  = *(Pxy+3) - '0';          //  | y


        // Echo command
        if(*module == 'e')
        {
            if(*(action+1) == 'n')
                echo = 1;
            else
                echo = 0;
        }
        // Module: gpio
        if(*module == 'g')
        {
            // If action is read, then
            if(*action == 'r')
            {
                uint8_t mode = INPUT;

                char * opt = strtok(0, " ");
                if(opt)
                {
                    if(*(opt+1) == 'u') //pull-up
                        mode |= PULLUP;
                    if(*(opt+1) == 'd') //pull-down
                        mode |= PULLDOWN;
                }

                gpio_mode(port, pin, mode);

                // get the value from the pin
                uint8_t value;
                uint8_t err = gpio_read(port, pin, &value);
                if(!err)
                    sprintf(container, "OK %d\r\n", value);
                else
                    sprintf(container, "ERROR %d\r\n", err); //TODO: Get better string error messages
            }
            // If action is write, then
            if (*action == 'w')
            {
                // retrieve the value to be written from
                // the next token
                char * value = strtok(0, " ");
                uint8_t err = gpio_write(port, pin, *value - '0');
                gpio_mode(port, pin, OUTPUT);

                if(!err)
                    sprintf(container, "OK\r\n");
                else
                    sprintf(container, "ERROR: %d\r\n", err); //TODO: Get better string error messages

            }
        }

        // Module: timer
        if(*module == 't')
        {
            // If action is to turn off
            if(*action == 'o')
            {
                uint8_t err = timer_off(port, pin);
                if(!err)
                    sprintf(container, "OK\r\n");
                else
                    sprintf(container, "ERROR: %d\r\n", err); //TODO: Get better string error messages
            }
            // If action is read
            if(*action == 'r')
            {
                uint32_t freq;
                uint16_t dc;
                uint8_t err = timer_read(port, pin, &freq, &dc);
                if(!err)
                    sprintf(container, "OK d:%d f:%d \r\n", dc, freq);
                else
                    sprintf(container, "ERROR: %d\r\n", err); //TODO: Get better string error messages
            }
            // If action is write
            if(*action == 'w')
            {
                // Get the frequency and duty cycle
                char * arg1 = strtok(0," ");
                char * arg2 = strtok(0," ");
                uint32_t freq;
                uint8_t dc;

                // Frequency format "f:<frequency value in Hz>"
                if(*arg1 == 'f')
                    freq = atoi(arg1 + 2);
                if(*arg2 == 'f')
                    freq = atoi(arg2 + 2);

                // Duty cycle format "d:<duty cycle in %>"
                if(*arg1 == 'd')
                    dc = atoi(arg1 + 2);
                if(*arg2 == 'd')
                    dc = atoi(arg2 + 2);

                uint8_t err = timer_write(port, pin, freq, dc);
                if(!err)
                    sprintf(container, "OK\r\n");
                else
                    sprintf(container, "ERROR: %d\r\n", err); //TODO: Get better string error messages

            }
        }

//        if(*module == 's')
//        {
//            char * interface = strtok(0," ");
//            // UART
//            if(*interface == 'u')
//            {
//                // Read
//                if(*action == 'r')
//                {
//                    serial_uart_read(container);
//                }
//                // Write
//                if(*action== 'w')
//                {
//                    // Get msg string
//                    char * arg = strtok(0," ");
//                    char * transfer_type = strtok(arg,":");
//                    if(*transfer_type == 's') // string transfer
//                    {
//                        serial_uart_write(arg + 2);
//                    }
//                    if(*transfer_type == 'b') // binary transfer
//                    {
//                        uint8_t transfer_size = atoi(strtok(0,":"));
//                        char * msg =
//                        serial_uart_write_binary(transfer_size, )
//                    }
//
//                }
//
//            }
//            if(*interface == 's')
//            {
//                // SPI
//            }
//            if(*interface == 'i')
//            {
//                // I2C
//            }
//        }

        usb_write(container);
        *container = 0;

    }
}
