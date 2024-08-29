#ifndef __USB_UTILS
#define __USB_UTILS

#include <stdint.h>

void usb_init();
void usb_read_until(char * buffer, uint8_t match_char);
void usb_write(char * msg);

#endif
