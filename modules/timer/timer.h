#ifndef __MODULES_TIMER
#define __MODULES_TIMER

#include <stdint.h>

// Defines
#define TIMER_SUCCESS              0
#define ERROR_TIMER_NOT_ACTIVE     1
#define ERROR_TIMER_NOT_AVAILABLE  2
#define ERROR_TIMER_DATA_NOT_READY 3

#define TA0_BASE_ADDR ((uint16_t *)0x340)
#define TA1_BASE_ADDR ((uint16_t *)0x380)
#define TA2_BASE_ADDR ((uint16_t *)0x400)

#define SMCLK_FREQ 15985904

// Types
typedef struct {
    enum {off, pwm, reading,
          data_ready} state;
    uint32_t   freq;
    uint16_t   dc;
    uint16_t * ctl;
    uint16_t   dt [8];
    uint16_t   ton[8];
    uint8_t    port;
    uint8_t    pin;
} timer_t;


// Functions
void    timer_init();
uint8_t timer_off  (uint8_t port, uint8_t pin);
uint8_t timer_read (uint8_t port, uint8_t pin, uint32_t * freq, uint16_t * dc);
uint8_t timer_write(uint8_t port, uint8_t pin, uint32_t   freq, uint16_t   dc);


#endif //__MODULES_TIMER
