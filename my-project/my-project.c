
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include "device_lib/hd44780-i2c.h"
#include "device_lib/ds1307.h"

#include "common_lib/utils.h"
#include "common_lib/usart.h"

#include "tm1638.h"

#include "sunrise.h"

static void gpio_setup(void)
{
	/* Enable GPIOA clock. */
	/* Manually: */
	// RCC_APB2ENR |= RCC_APB2ENR_IOPCEN;
	/* Using API functions: */
	rcc_periph_clock_enable(RCC_GPIOB);

	/* Set GPIO5 (in GPIO port A) to 'output push-pull'. */
	/* Manually: */
	// GPIOA_CRH = (GPIO_CNF_OUTPUT_PUSHPULL << (((5 - 8) * 4) + 2));
	// GPIOA_CRH |= (GPIO_MODE_OUTPUT_2_MHZ << ((5 - 8) * 4));
	/* Using API functions: */
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_2_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, GPIO1 | GPIO12|GPIO13|GPIO14);
}
static void i2c_setup(void)
{
	/* Enable GPIOA clock. */
	/* Manually: */
	// RCC_APB2ENR |= RCC_APB2ENR_IOPCEN;
	/* Using API functions: */
	//rcc_periph_clock_enable(RCC_GPIOB);

	/* Enable clocks for I2C2 and AFIO. */
	rcc_periph_clock_enable(RCC_I2C1);
	rcc_periph_clock_enable(RCC_AFIO);

	/* Set alternate functions for the SCL and SDA pins of I2C2. */
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_ALTFN_OPENDRAIN,
		      GPIO_I2C1_SCL | GPIO_I2C1_SDA);


	/* Disable the I2C before changing any configuration. */
	i2c_peripheral_disable(I2C1);

	i2c_reset(I2C1);

	/* APB1 is running at 36MHz. */
	i2c_set_clock_frequency(I2C1, I2C_CR2_FREQ_36MHZ);

	/* 400KHz - I2C Fast Mode */
	i2c_set_fast_mode(I2C1);

	/*
	 * fclock for I2C is 36MHz APB2 -> cycle time 28ns, low time at 400kHz
	 * incl trise -> Thigh = 1600ns; CCR = tlow/tcycle = 0x1C,9;
	 * Datasheet suggests 0x1e.
	 */
	i2c_set_ccr(I2C1, 0x1e);

	/*
	 * fclock for I2C is 36MHz -> cycle time 28ns, rise time for
	 * 400kHz => 300ns and 100kHz => 1000ns; 300ns/28ns = 10;
	 * Incremented by 1 -> 11.
	 */
	i2c_set_trise(I2C1, 0x0b);

	/* If everything is configured -> enable the peripheral. */
	i2c_peripheral_enable(I2C1);
}

/* Your timezone, will be stored in DS1307 NVRAM.
   Sorry Iran and India, only integer hours here */
int TZ = 3;

/* Europe/Moscow, should be stored in DS1307 NVRAM */
float latitude = 37.617222;
float longitude = 55.755833;
void display_sunrise(i2c_device *clock, struct tm1638 *led, hd44780_device *lcd);

void display_sunrise(i2c_device *clock, struct tm1638 *led, hd44780_device *lcd)
{
	/* Adjust time for timezone shift */
	struct tm tm = ds1307_read_date(clock);
	hd44780_go_to(lcd, 1, 0);
	hd44780_print(lcd, "PACCBET");

	tm.tm_hour += TZ;

	time_t now = mktime(&tm);

	/* If it's local afternoon, then we're looking for times for tomorrow */
	if( tm.tm_sec+tm.tm_min*60+tm.tm_hour*3600 > 12*3600) {
		// for tomorrow
		tm.tm_mday+=1;
	}
	/* adjust time if tomorrow is 32 of March :) */
	mktime(&tm);

	uint32_t secs_rise=0, secs_set=0;

	calc_sun_times(&tm, longitude, latitude, &secs_rise, &secs_set);
	tm.tm_sec = secs_rise;
	tm.tm_hour = TZ;
	tm.tm_min = 0;

	time_t rise = mktime(&tm);

	int secs_until_sunrise = rise-now;
	int mins_until_sunrise = secs_until_sunrise/60;
	int ss = secs_until_sunrise%60;

	char until_sunrise[32]={0};
	sprintf(until_sunrise, "%03d.%02d   ", mins_until_sunrise, ss);
	tm1638_write_string(led, until_sunrise);
}

int main(void)
{
	int i;

	gpio_setup();
	i2c_setup();
	usart1_init(115200);
	usart1_printf("[31;1mFANCY[32;1mCLOCK[0m starting:%d\r\n",1);

	i2c_scan_bus(I2C1);

	setup_delay_timer(TIM2);

	hd44780_device lcd;
	hd44780_init(&lcd, I2C1, 0x27, TIM2);

	i2c_device *clock = ds1307_init(I2C1);

	struct tm1638 led_display;

	tm1638_init(&led_display, GPIOB, GPIO14, GPIO13, GPIO12);

	tm1638_write_string(&led_display, "0.1.2.3    ");

	struct tm now = {
		.tm_year = 2021-1900,
		.tm_mon = 4-1, /*  */
		.tm_mday = 7,
		.tm_hour = 21,
		.tm_min = 28,
		.tm_sec = 0
	};

//	ds1307_set_date(clock, &now);

	hd44780_print(&lcd, "hello");
	int backlight =0;

	uint8_t ch=0;
	uint8_t led_br=0;

	/* Blink the LED (PA5) on the board. */
	while (1) {
		char out[40]={0};
		hd44780_go_to(&lcd, 0, 0);

		now = ds1307_read_date(clock);

		now.tm_hour += TZ;

		mktime(&now);

		hd44780_backlight(&lcd, backlight);


		char datestr[40]={0}, timestr[40]={0};
		strftime(datestr, sizeof(datestr), "%y%m%d", &now);
		strftime(timestr, sizeof(timestr), "%H:%M:%S ", &now);

		hd44780_print(&lcd, datestr);
		hd44780_go_to(&lcd, 0, 7);
		hd44780_print(&lcd, timestr);
/*
		sprintf(out, "%02x", ch);
		hd44780_go_to(&lcd, 0, 14);
		hd44780_print(&lcd, out);

		out[0]=ch++; out[1]=0;
		hd44780_go_to(&lcd, 1, 15);
		hd44780_print(&lcd, out);
		if(ch == 0) {
			backlight = !backlight;
		}
*/
		display_sunrise(clock, &led_display, &lcd);


		uint32_t but = tm1638_get_buttons(&led_display);
//		usart1_printf("Buttons: %08x\r\n", but);
		if(but&1) {
			led_br++;
			led_br &=0x07;
			usart1_printf("New brightness: %08x\r\n", led_br);
		}

		if(but&2) {
			backlight = !backlight;
		}


		tm1638_set_brightness(&led_display, 0x08|led_br);


		/* Using API function gpio_toggle(): */
//		gpio_toggle(GPIOB, GPIO1);	/* LED on/off */
		for (i = 0; i < 100000; i++)	/* Wait a bit. */
			__asm__("nop");
	}

	return 0;
}
