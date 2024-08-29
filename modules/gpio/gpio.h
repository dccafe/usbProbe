#ifndef __MODULES_GPIO
#define __MODULES_GPIO

#include <stdint.h>

#define isEven(n) !(port & 0x01)
#define isOdd(n)   (port & 0x01)

#define PORT_BASE_ADDR (uint8_t *)0x200

#define GPIO            0x00
#define FUNC1           0x10
#define FUNC2           0x20
#define FUNC3           0x30
#define OUTPUT          0x00
#define INPUT           0x04
#define PULLUP          0x02
#define PULLDOWN        0x01

void gpio_mode(uint8_t port, uint8_t pin, uint8_t mode);

uint8_t gpio_read (uint8_t port, uint8_t pin, uint8_t * value);
uint8_t gpio_write(uint8_t port, uint8_t pin, uint8_t  value);


#endif //__MODULES_GPIO
