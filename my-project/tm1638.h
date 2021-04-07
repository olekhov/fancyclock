/* library to drive TM1638 LED displays */
#pragma once

#include <stdint.h>
#include <stddef.h>

struct tm1638 {
    uint32_t port;
    uint16_t strobe, data, clock, curaddr;
};

enum {
	RED_LED=0x01, GREEN_LED=0x02
};

void tm1638_init(struct tm1638 *t, uint32_t port, uint16_t strobe, uint16_t data, uint16_t clock);
void tm1638_clear(struct tm1638 *t);
void tm1638_set_led(struct tm1638 *t,uint8_t pos, uint8_t color);
void tm1638_raw_write(struct tm1638 *t,uint8_t pos, uint8_t data);
void tm1638_write_char(struct tm1638*t, uint8_t ch);
void tm1638_write_string(struct tm1638*t, const char *s);
uint32_t tm1638_get_buttons(struct tm1638*t);
void tm1638_send_byte(struct tm1638*t,uint8_t b);
void tm1638_set_brightness(struct tm1638*t,uint8_t b);

