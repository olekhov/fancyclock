PROJECT = fancyclock
BUILD_DIR = bin

SHARED_DIR = ../libs
CFLAGS +=-DLED_PORT=GPIOB -DLED_PIN=GPIO1 -g
CFILES = my-project.c device_lib/hd44780-i2c.c common_lib/utils.c common_lib/i2c.c common_lib/usart.c
CFILES += device_lib/ds1307.c device_lib/ds_common.c
CFILES += tm1638.c sunrise.c

# TODO - you will need to edit these two lines!
DEVICE=stm32f103cbt
OOCD_FILE = my.openocd.cfg

# You shouldn't have to edit anything below here.
VPATH += $(SHARED_DIR)
INCLUDES += $(patsubst %,-I%, . $(SHARED_DIR))
OPENCM3_DIR=../libopencm3

include $(OPENCM3_DIR)/mk/genlink-config.mk
include ../rules.mk
include $(OPENCM3_DIR)/mk/genlink-rules.mk

install: ${PROJECT}.bin
	st-flash write ${PROJECT}.bin 0x8000000
