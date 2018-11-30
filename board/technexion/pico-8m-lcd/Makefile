#
# Copyright 2018 Technexion Ltd.
#
# SPDX-License-Identifier:      GPL-2.0+
#

obj-y += pico-8m-lcd.o

ifdef CONFIG_SPL_BUILD
obj-y += pico-8m-lcd_spl.o
obj-y += ddr/ddr_init.o ddr/ddrphy_train.o ddr/helper.o
endif
