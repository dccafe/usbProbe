#include <string.h>

#include "driverlib.h"

#include "USB_config/descriptors.h"
#include "USB_API/USB_Common/device.h"
#include "USB_API/USB_Common/usb.h"                 // USB-specific functions
#include "USB_API/USB_CDC_API/UsbCdc.h"
#include "USB_app/usbConstructs.h"

#include "hal.h"


void usb_init()
{
    // Minimum Vcore setting required for the USB API is PMM_CORE_LEVEL_2 .
    PMM_setVCore(PMM_CORE_LEVEL_2);

    USBHAL_initPorts();           // Config GPIOS for low-power (output low)
    USBHAL_initClocks(8000000);   // Config clocks. MCLK=SMCLK=FLL=8MHz; ACLK=REFO=32kHz
    USB_setup(TRUE, TRUE); // Init USB & events; if a host is present, connect

    __enable_interrupt();  // Enable interrupts globally

    while(USB_getConnectionState() != ST_ENUM_ACTIVE);

}

extern uint8_t echo;

void usb_read_until(char * buffer, uint8_t match_char)
{
    uint8_t nBytes = 0;
    uint8_t c;
    do
    {
        // Get char
        if(USBCDC_receiveDataInBuffer(&c, 1, 0))
        {
            // save it to the buffer
            if(c == 0x7F) //(del)
                nBytes--;
            else
                buffer[nBytes++] = c;
            // and echo back to the terminal (consider removing)
            if(echo)
                USBCDC_sendDataAndWaitTillDone(&c, 1, 0, 0xFFFFFFFF);

        }
    } while(c != match_char);
    // Add end of string
    buffer[nBytes] = 0;

}

void usb_write(char * msg)
{
    USBCDC_sendDataAndWaitTillDone((uint8_t *)msg, strlen(msg), 0, 0xFFFFFFFF);
}
// Global flags set by events
volatile uint8_t bCDCDataReceived_event = 0;  // Flag set by event handler to
                                               // indicate data has been
                                               // received into USB buffer


#pragma vector = UNMI_VECTOR
__interrupt void UNMI_ISR (void)
{
    switch (__even_in_range(SYSUNIV, SYSUNIV_BUSIFG ))
    {
        case SYSUNIV_NONE:
            __no_operation();
            break;
        case SYSUNIV_NMIIFG:
            __no_operation();
            break;
        case SYSUNIV_OFIFG:
            UCS_clearFaultFlag(UCS_XT2OFFG);
            UCS_clearFaultFlag(UCS_DCOFFG);
            SFR_clearInterrupt(SFR_OSCILLATOR_FAULT_INTERRUPT);
            break;
        case SYSUNIV_ACCVIFG:
            __no_operation();
            break;
        case SYSUNIV_BUSIFG:
            // If the CPU accesses USB memory while the USB module is
            // suspended, a "bus error" can occur.  This generates an NMI.  If
            // USB is automatically disconnecting in your software, set a
            // breakpoint here and see if execution hits it.  See the
            // Programmer's Guide for more information.
            SYSBERRIV = 0; // clear bus error flag
            USB_disable(); // Disable
    }
}
