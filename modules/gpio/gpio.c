#include <stdint.h>
#include "gpio.h"

// Set mode: OUTPUT, INPUT, with PULLUP or PULLDOWN, in GPIO mode or FUNC[1/2/3]
void gpio_mode(uint8_t port, uint8_t pin, uint8_t mode)
{
    uint8_t *baseAddr, *PxOUT, *PxDIR, *PxREN, *PxSEL, mask;
    mask = (0x01 << pin);

    baseAddr = (uint8_t *)( PORT_BASE_ADDR + ( (port-1) >> 1 ) * 0x20 + isEven(port) );

    PxOUT  = baseAddr + 0x2;
    PxDIR  = baseAddr + 0x4;
    PxREN  = baseAddr + 0x6;
    PxSEL  = baseAddr + 0xA;                // aka PSEL0

    if( mode & INPUT )                      // If it is an input
        *PxDIR &= ~mask;                    // Clear DIR bit
    else                                    // else (if it is an output)
        *PxDIR |=  mask;                    // Set DIR bit

    if( mode & (PULLDOWN | PULLUP) )        // Does it requires a resistor?
    {
        *PxREN |=  mask;                    // If so, enable resitor
        if (mode & PULLUP)                  // and select either
            *PxOUT |=  mask;                // pull-up
        else                                // or
            *PxOUT &= ~mask;                // pull-down
    }                                       //
    else                                    // If no resistor required,
        *PxREN &= ~mask;                    // clear REN bit

    if (mode & FUNC1)                       // If dedicated function is selected
        *PxSEL |=  mask;                    // set SEL bit
    else                                    // else
        *PxSEL &= ~mask;                    // clear SEL bit

}

uint8_t gpio_write(uint8_t port, uint8_t pin, uint8_t value)
{
    uint8_t * POUT;
    uint8_t mask = 1 << pin;
    POUT = (uint8_t *)(PORT_BASE_ADDR
                  + ((port-1)>>1)*0x20      // Base addr
                  + isEven(port) + 2);      // Offset

    if(value)
        *POUT |=  mask;
    else
        *POUT &= ~mask;

    return 0;
}

uint8_t gpio_read(uint8_t port, uint8_t pin, uint8_t * value)
{
    uint8_t * PIN;
    PIN = (uint8_t *)(PORT_BASE_ADDR
            + ((port-1) >> 1)*0x20          // Base Address
            + isEven(port));                // Offset

    *value = *PIN & (0x01 << pin);

    return 0;
}
