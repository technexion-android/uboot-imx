// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2024 TechNexion Ltd.
 *
 */
#ifndef FB_ANDROID_DTBO_H__
#define FB_ANDROID_DTBO_H__

#include <linux/compat.h>
#ifdef CONFIG_AVB_SUPPORT
#include <dt_table.h>
#endif

enum {
	DEV_SETUP_UNKNOWN = 0,
	DEV_SETUP_VIN,
	DEV_SETUP_VOUT,
	DEV_SETUP_AIN,
	DEV_SETUP_AOUT,
	DEV_SETUP_NFC,

	DEV_SETUP_TOTAL,
};

typedef struct DEV_SETUP_T {
	u8 type;
	const char *dtoverlay;
	const char *dev_conf;
} dev_setup_t;

int _add_dt_overlay(struct dt_table_header *dt_img, const u32 fdt_addr);
void _run_fw_envcmd(void);

#endif //#ifndef FB_ANDROID_DTBO_H__