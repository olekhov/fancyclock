/*
 * Library for TM1638 LED 7-segment display driver
 */

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <string.h>
#include "tm1638.h"

/*
 *  ___0___
 * |       |
 * 5       1
 * |___6___|
 * |       |
 * 4       2
 * |___3___| (*) 7
 *
 *
 */

static const uint8_t font[]=
    { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, // 0..9
      0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71,    // A B C D E F
    };

#define  DISPLAY_CONTROL  0x80
#define  ADDRESS_SET      0xC0

#define  DATA_WRITE       0x40
#define  DATA_READ        0x42
#define  DATA_ADDRFIX     0x44

static uint8_t FontChar(uint8_t ch)
{
	if(ch>='0' && ch<='9') return font[ch-'0'];
	if(ch>='a' && ch<='f') return font[ch-'a'+10];
	if(ch>='A' && ch<='F') return font[ch-'A'+10];
	if(ch=='-') return 0x40;
	return 0;
}

static void delay(uint32_t some)
{
	uint32_t i;
	for (i = 0; i < some; i++)
		__asm__("nop");
}

static const uint32_t delayscale=1;

static void send_byte(struct tm1638*t,uint8_t b)
{
	uint8_t i;
	gpio_set_mode(t->port, GPIO_MODE_OUTPUT_2_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, t->data);

	for(i=0;i<8;i++)
	{
		gpio_clear(t->port, t->clock);
		if(b&1)
			gpio_set(t->port, t->data);
		else
			gpio_clear(t->port, t->data);

		delay(delayscale);
		gpio_set(t->port, t->clock);
		delay(delayscale);
		b>>=1;
	}
}

static uint8_t receive(struct tm1638*t)
{
	uint8_t temp = 0;

	// Pull-up on
	gpio_set_mode(t->port, GPIO_MODE_INPUT,
		      GPIO_CNF_INPUT_PULL_UPDOWN, t->data);
	gpio_set(t->port, t->data);

	for (int i = 0; i < 8; i++) {
		temp >>= 1;

		gpio_clear(t->port, t->clock);

		if (gpio_get(t->port, t->data)) {
			temp |= 0x80;
		}

		gpio_set(t->port, t->clock);
	}

	// Pull-up off
	gpio_set_mode(t->port, GPIO_MODE_OUTPUT_2_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, t->data);
	gpio_clear(t->port, t->data);


	return temp;
}

static void send_batch(struct tm1638 *t, uint8_t cmd, const uint8_t *data, uint32_t data_count)
{
	uint32_t i;

	gpio_clear(t->port, t->strobe);

	send_byte(t, cmd);
	for(i=0;i<data_count;i++)
		send_byte(t, data[i]);

	gpio_set(t->port, t->strobe);
}

/*  Задать пины: STB0, DIO, CLK */
void tm1638_init(struct tm1638 *t, uint32_t port, uint16_t strobe, uint16_t data, uint16_t clock)
{
	memset(t, 0, sizeof(struct tm1638));
	t->port = port;
	t->strobe = strobe;
	t->data = data;
	t->clock = clock;
	gpio_set(t->port, t->strobe);
	t->curaddr = 0;

	send_batch(t, DATA_WRITE, NULL, 0);
	send_batch(t, DISPLAY_CONTROL|0x0F, NULL, 0);
	tm1638_clear(t);
}

void tm1638_clear(struct tm1638 *t)
{
	uint8_t clear[16]={0};
	send_batch(t, ADDRESS_SET|0, clear, 16);
	send_batch(t, ADDRESS_SET|0, NULL, 0);
}

void tm1638_set_led(struct tm1638 *t,uint8_t pos, uint8_t color)
{
	send_batch(t, ADDRESS_SET|(pos*2+1), &color, 1);
}

void tm1638_raw_write(struct tm1638 *t,uint8_t pos, uint8_t data)
{
	send_batch(t, ADDRESS_SET|pos, &data, 1);
}

uint32_t tm1638_get_buttons(struct tm1638*t)
{
	uint32_t keys = 0;

	gpio_clear(t->port, t->strobe);
	send_byte(t, 0x42);

	for (int i = 0; i < 4; i++) {
		keys |= receive(t) << i;
	}
	gpio_set(t->port, t->strobe);

	return keys;
}

void tm1638_write_char(struct tm1638*t, uint8_t ch)
{
	uint8_t d;
	d=FontChar(ch);
	send_batch(t, ADDRESS_SET|t->curaddr,&d,1);
	t->curaddr+=2;
}

void tm1638_write_string(struct tm1638*t, const char *s)
{
	int i=0,bp=0;
	uint8_t sendbuf[16]={0};
	t->curaddr = 0;

	while(s[i]!=0) {

		uint8_t ch=s[i];

		ch=FontChar(s[i]);
		if(s[i+1] =='.') {ch |= 0x80; i++; }
#if 0
		sendbuf[bp]=ch; bp+=2;
		if(bp>=16) break;
#else
		tm1638_raw_write(t, t->curaddr, ch);
		t->curaddr += 2;
#endif
		i++;
	}
//	send_batch(t, ADDRESS_SET|0, sendbuf, bp);
}

void tm1638_set_brightness(struct tm1638*t,uint8_t b)
{
	send_batch(t, DISPLAY_CONTROL| (b&0x0f), NULL, 0);
}
