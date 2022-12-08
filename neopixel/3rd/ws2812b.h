#ifndef WS2812B_H
#define WS2812B_H
#include <stdint.h>

void ws2812b_setup(void);
void ws2812b_send_pixels(uint32_t *data, uint8_t len);

#endif
