# Fancyclock

My son continuously asks "how much minutes left until the night ends".

This fancy clock shows the remaining minutes (and seconds) until nearest sunrise.

Used hardware:

* STM32F103CBT6 MapleMini clone (Baite version)
* TM1638 LED 8-digit display (with buttons and two-color LEDS)
* 1608 LCD display with I2C driver
* TinyRTC module - DS1307 clock and AT21 EEPROM

# Building

I build on ArtixLinux.

Toolchain:
* community/arm-none-eabi-gcc 10.2.0-1
* community/arm-none-eabi-binutils 2.36.1-2
* community/arm-none-eabi-newlib 4.1.0-1

Programmer:
* ST-Link V2 clone and community/stlink 1.6.1-1

Compiling:
```
$ make -j4 -C libopencm3
$ make -C my-project
```

Flashing:
```
$ make -C my-project flash
```

