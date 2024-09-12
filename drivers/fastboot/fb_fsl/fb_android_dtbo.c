#include "fb_android_dtbo.h"
#include <asm/mach-imx/sys_proto.h>
#include <env.h>
#include <common.h>
#include <image.h>

#define EXT_DTBO_SIZE(t)		(sizeof(t)/sizeof(t[0]))
#define NO_OVERLAY				(-1)

#define BOOTARGS_APPEND_DTBO	(1)


//
// Sync the order of _ext_dtbo with BoardConfig.mk
//
#if(defined(CONFIG_TARGET_TEK_IMX8MP))
static dev_setup_t _dev_setup_tek_8mp[] = {
	{ DEV_SETUP_VOUT, "hdmi2mipi-tc358743", NULL },
	{ DEV_SETUP_VIN,  "vizionlink-tevi-ov5640", NULL },
	{ DEV_SETUP_VIN,  "vizionlink-tevi-ap1302", "ar0144" },
	{ DEV_SETUP_VIN,  "vls", NULL },
};
#endif

#if(defined(CONFIG_TARGET_TEP_IMX8MP))
static dev_setup_t _dev_setup_tep_8mp[] = {
	{ DEV_SETUP_VOUT, "hdmi2mipi-tc358743", NULL },
	{ DEV_SETUP_VOUT, "lvds-vl10112880", NULL },
	{ DEV_SETUP_VOUT, "lvds-vl15010276", NULL },
	{ DEV_SETUP_VOUT, "lvds-vl156192108", NULL },
	{ DEV_SETUP_VOUT, "lvds-vl215192108", NULL },
	{ DEV_SETUP_VOUT, "vizionpanel-vl10112880", NULL },
	{ DEV_SETUP_VOUT, "vizionpanel-vl15010276", NULL },
	{ DEV_SETUP_VOUT, "vizionpanel-vl15613676", NULL },
	{ DEV_SETUP_VIN,  "vizionlink-tevi-ov5640", NULL },
	{ DEV_SETUP_VIN,  "vizionlink-tevi-ap1302", "ar0144" },
	{ DEV_SETUP_VIN,  "vls", NULL },
};
#endif

#if(defined(CONFIG_TARGET_AXON_IMX8MP))
static dev_setup_t _dev_setup_axon_8mp[] = {
	{ DEV_SETUP_VOUT, "lvds-vl10112880", NULL },
	{ DEV_SETUP_VOUT, "lvds-vl15613676", NULL },
	{ DEV_SETUP_VOUT, "lvds-vl215192108", NULL },
	{ DEV_SETUP_VOUT, "vizionpanel-vl10112880", NULL },
	{ DEV_SETUP_VOUT, "vizionpanel-vl15010276", NULL },
	{ DEV_SETUP_VOUT, "vizionpanel-vl15613676", NULL },
	{ DEV_SETUP_VIN, "tevi-ov5640", NULL },
	{ DEV_SETUP_VIN, "tevi-ap1302", "ar0144" },
	{ DEV_SETUP_VIN, "vizionlink-tevi-ap1302", "ar0144" },
	{ DEV_SETUP_VIN, "tevs", NULL },
	{ DEV_SETUP_VIN, "vls", NULL },
};
#endif

#if(defined(CONFIG_TARGET_EDM_G_IMX8MP))
static dev_setup_t _dev_setup_edm_g_8mp[] = {
	{ DEV_SETUP_VOUT, "lvds-vl10112880", NULL },
	{ DEV_SETUP_VOUT, "lvds-vl15613676", NULL },
	{ DEV_SETUP_VOUT, "lvds-vl156192108", NULL },
	{ DEV_SETUP_VOUT, "lvds-vl215192108", NULL },
	{ DEV_SETUP_VIN, "tevi-ov5640", NULL },
	{ DEV_SETUP_VIN, "tevi-ap1302", "ar0144" },
	{ DEV_SETUP_VIN, "vizionlink-tevi-ov5640", NULL },
	{ DEV_SETUP_VIN, "vizionlink-tevi-ap1302", "ar0144" },
	{ DEV_SETUP_VIN, "tevs", NULL },
	{ DEV_SETUP_VIN, "vls", NULL },
};
static dev_setup_t _dev_setup_edm_g_8mp_wizard[] = {
	{ DEV_SETUP_VOUT, "vizionpanel-vl10112880", NULL },
	{ DEV_SETUP_VOUT, "vizionpanel-vl15010276", NULL },
	{ DEV_SETUP_VOUT, "vizionpanel-vl15613676", NULL },
	{ DEV_SETUP_VOUT, "vizionpanel-vl215192108", NULL },
};
#endif

static dev_setup_t _dev_setup_edm_g_8mn[] = {
	{ DEV_SETUP_VOUT, "lvds-vl10112880", NULL },
	{ DEV_SETUP_VOUT, "lvds-vl15613676", NULL },
	{ DEV_SETUP_VOUT, "lvds-vl215192108", NULL },
	{ DEV_SETUP_VOUT, "mipi2hdmi-adv7535", NULL },
	{ DEV_SETUP_VIN, "tevi-ov5640", NULL },
	{ DEV_SETUP_VIN, "tevi-ap1302", "ar0144" },
};

#if(defined(CONFIG_TARGET_PICO_IMX8MQ))
static dev_setup_t _dev_setup_pico_8mq[] = {
	{ DEV_SETUP_VOUT, "ili9881c", NULL },
	{ DEV_SETUP_VIN, "tevi-ov5640", NULL },
};

static dev_setup_t _dev_setup_pico_8mq_pi[] = {
	{ DEV_SETUP_VOUT, "dual", NULL },
	{ DEV_SETUP_VIN, "tevs", "tevs" },
};

static dev_setup_t _dev_setup_pico_8mq_wizard[] = {
	{ DEV_SETUP_VOUT, "mipi2hdmi-adv7535", NULL },
};
#endif

#if(defined(CONFIG_TARGET_EDM_G_IMX8MM))
static dev_setup_t _dev_setup_edm_g_8mm[] = {
	{ DEV_SETUP_VOUT, "sn65dsi84-vl10112880", NULL },
	{ DEV_SETUP_VOUT, "sn65dsi84-vl15613676", NULL },
	{ DEV_SETUP_VOUT, "sn65dsi84-vl215192108", NULL },
	{ DEV_SETUP_VIN, "tevi-ov5640", NULL },
	{ DEV_SETUP_VIN, "tevi-ap1302", "ar0144" },
	{ DEV_SETUP_VIN, "hdmi2mipi-tc358743", NULL },
	{ DEV_SETUP_VIN, "vizionlink-tevi-ap1302", "ar0144" },
	{ DEV_SETUP_VIN, "tevs", NULL },
	{ DEV_SETUP_VIN, "vls", NULL },
};
#endif

#if(defined(CONFIG_TARGET_PICO_IMX8MM))
static dev_setup_t _dev_setup_pico_8mm[] = {
	{ DEV_SETUP_VOUT, "ili9881c", NULL },
	{ DEV_SETUP_VIN, "tevi-ov5640", NULL },
};

static dev_setup_t _dev_setup_pico_8mm_pi[] = {
	{ DEV_SETUP_VIN, "tevi-ap1302", "ar0144" },
};

static dev_setup_t _dev_setup_pico_8mm_wizard[] = {
	{ DEV_SETUP_VOUT, "mipi2hdmi-adv7535", NULL },
	{ DEV_SETUP_VOUT, "sn65dsi84-vl10112880", NULL },
	{ DEV_SETUP_VOUT, "sn65dsi84-vl15613676", NULL },
	{ DEV_SETUP_VOUT, "sn65dsi84-vl215192108", NULL },
};
#endif

static  int _get_dev_setup(const char *dtbo_token, dev_setup_t *dev_setup, size_t *dev_setup_cnt) {
	dev_setup_t *__dev_setup = NULL;
	size_t __dev_setup_cnt = 0;
	dev_setup_t *__dev_setup_ext = NULL;
	size_t __dev_setup_ext_cnt = 0;
	int idx = NO_OVERLAY;

	if(dtbo_token == NULL) {
		goto _exit_get_dtbo_index;
	}

	if(is_imx8mp()) {
#if(defined(CONFIG_TARGET_TEK_IMX8MP))
		__dev_setup = _dev_setup_tek_8mp;
		__dev_setup_cnt = EXT_DTBO_SIZE(_dev_setup_tek_8mp);
#elif(defined(CONFIG_TARGET_TEP_IMX8MP))
		__dev_setup = _dev_setup_tep_8mp;
		__dev_setup_cnt = EXT_DTBO_SIZE(_dev_setup_tep_8mp);
#elif(defined(CONFIG_TARGET_AXON_IMX8MP))
		__dev_setup = _dev_setup_axon_8mp;
		__dev_setup_cnt = EXT_DTBO_SIZE(_dev_setup_axon_8mp);
#elif(defined(CONFIG_TARGET_EDM_G_IMX8MP))
		__dev_setup = _dev_setup_edm_g_8mp;
		__dev_setup_cnt = EXT_DTBO_SIZE(_dev_setup_edm_g_8mp);
		if (strcmp("wizard", env_get("baseboard")) == 0)
		{
			__dev_setup_ext = _dev_setup_edm_g_8mp_wizard;
			__dev_setup_ext_cnt = EXT_DTBO_SIZE(_dev_setup_edm_g_8mp_wizard);
		}
#endif
	} else if(is_imx8mm()) {
#if(defined(CONFIG_TARGET_EDM_G_IMX8MM))
		__dev_setup = _dev_setup_edm_g_8mm;
		__dev_setup_cnt = EXT_DTBO_SIZE(_dev_setup_edm_g_8mm);
#elif(defined(CONFIG_TARGET_PICO_IMX8MM))
		__dev_setup = _dev_setup_pico_8mm;
		__dev_setup_cnt = EXT_DTBO_SIZE(_dev_setup_pico_8mm);
		if (strcmp("pi", env_get("baseboard")) == 0)
		{
			__dev_setup_ext = _dev_setup_pico_8mm_pi;
			__dev_setup_ext_cnt = EXT_DTBO_SIZE(_dev_setup_pico_8mm_pi);
		}
		if (strcmp("wizard", env_get("baseboard")) == 0)
		{
			__dev_setup_ext = _dev_setup_pico_8mm_wizard;
			__dev_setup_ext_cnt = EXT_DTBO_SIZE(_dev_setup_pico_8mm_wizard);
		}
#endif
	} else if(is_imx8mq()) {
#if(defined(CONFIG_TARGET_PICO_IMX8MQ))
		__dev_setup = _dev_setup_pico_8mq;
		__dev_setup_cnt = EXT_DTBO_SIZE(_dev_setup_pico_8mq);
		if (strcmp("pi", env_get("baseboard")) == 0)
		{
			__dev_setup_ext = _dev_setup_pico_8mq_pi;
			__dev_setup_ext_cnt = EXT_DTBO_SIZE(_dev_setup_pico_8mq_pi);
		}
		if (strcmp("wizard", env_get("baseboard")) == 0)
		{
			__dev_setup_ext = _dev_setup_pico_8mq_wizard;
			__dev_setup_ext_cnt = EXT_DTBO_SIZE(_dev_setup_pico_8mq_wizard);
		}
#endif
	} else if(is_imx8mn()) {
		__dev_setup = _dev_setup_edm_g_8mn;
		__dev_setup_cnt = EXT_DTBO_SIZE(_dev_setup_edm_g_8mn);
	}
	else {
		goto _exit_get_dtbo_index;
	}
	
	for(idx = 0; idx < __dev_setup_cnt; ++idx) {
		//printf("%s - __dev_setup[%d].dtoverlay = %s\n", __func__, idx, __dev_setup[idx].dtoverlay);
		if(strcmp(dtbo_token, __dev_setup[idx].dtoverlay) == 0) {
			goto _exit_get_dtbo_index;
		}
	}
	if (__dev_setup_ext != NULL) {
		for(idx = 0; idx < __dev_setup_ext_cnt; ++idx) {
			//printf("%s - __dev_setup_ext[%d].dtoverlay = %s\n", __func__, idx, __dev_setup_ext[idx].dtoverlay);
			if(strcmp(dtbo_token, __dev_setup_ext[idx].dtoverlay) == 0) {
				idx += __dev_setup_cnt;
				goto _exit_get_dtbo_index;
			}
		}
	}

	idx = NO_OVERLAY;

_exit_get_dtbo_index:

	if(idx != NO_OVERLAY) {
		if ( idx < __dev_setup_cnt)
			*dev_setup = __dev_setup[idx];
		else
			*dev_setup = __dev_setup_ext[idx-__dev_setup_cnt];
	}

	return(idx);
}

static  int _dtoverlay_bootargs_add(dev_setup_t dev_setup) {
#if(BOOTARGS_APPEND_DTBO)
	switch(dev_setup.type) {
		case DEV_SETUP_VIN: {
			char buf[128] = {'\0'}, conf[128]= {'\0'};
			char *bootarg = NULL;

			//printf("---->>> %s - dev_conf: %s\n", __func__, dev_setup.dev_conf);
			if((dev_setup.dev_conf != NULL) && (dev_setup.dev_conf[0] != '\0')) {
				snprintf(conf, sizeof(buf)-1, "androidboot.camera.layout=%s", dev_setup.dev_conf);

				bootarg = env_get("append_bootargs");
				if (bootarg == NULL) {
					snprintf(buf, sizeof(buf)-1, "%s", conf);
				} else if(strstr(bootarg, conf)) {
					snprintf(buf, sizeof(buf)-1, "%s", bootarg);
				} else {
					snprintf(buf, sizeof(buf)-1, "%s %s", bootarg, conf);
				}
				printf("ANDROID: set append_bootargs: %s\n", buf);
				env_set("append_bootargs", buf);
			}
	 		break;
		}
		default:
			break;
	}
#endif
	return(0);
}

int _add_dt_overlay(struct dt_table_header *dt_img, const u32 fdt_addr) {
	u32 dtbo_addr = fdt_addr + 0xF0000;
	struct dt_table_entry *dt_entry_overlay = NULL;
	int dtbo_idx = NO_OVERLAY;
	dev_setup_t dev_setup;
	u32 fdt_overlay_size = 0;
	int ret = -1;
	char dtoverlay[32];
	char *dtoverlay_ptr = NULL, *dtbo_token = NULL;
	//u8 dt_entry_cnt = be32_to_cpu(dt_img->dt_entry_count);
	u32 dt_entry_size = be32_to_cpu(dt_img->dt_entry_size);
	u32 dt_entry_offset = be32_to_cpu(dt_img->dt_entries_offset);

	if (dt_img == NULL) {
		printf("%s - Invalid pointer of dt images\n", __func__);
		goto _exit_add_dt_overlay;
	}

	dtoverlay_ptr = env_get("dtoverlay");
	if(dtoverlay_ptr == NULL) {
		goto _exit_add_dt_overlay;
	}

	sprintf(dtoverlay, "%s", dtoverlay_ptr);

	dtbo_token = strtok(dtoverlay, " ");
	while(dtbo_token != NULL) {
		dtbo_idx = _get_dev_setup(dtbo_token, &dev_setup, NULL);
		//printf("%s - dtbo_token: %s, dtbo_idx: %d, dev_setup.dtoverlay: %s \n", __func__, dtbo_token, dtbo_idx, dev_setup.dtoverlay);

		if(dtbo_idx == NO_OVERLAY) {
			//printf("%s - Invalid dev_setup (index: %d)\n", __func__, dtbo_idx);
			goto next_dtbo;
		}

		dt_entry_overlay = (struct dt_table_entry *)((ulong)dt_img + dt_entry_offset + ((u32)(dtbo_idx+1) * dt_entry_size));
		fdt_overlay_size = be32_to_cpu(dt_entry_overlay->dt_size);
		if (fdt_overlay_size < 0) {
			printf("ERROR: Invalid size of fdt overlay\n");
			goto next_dtbo;
		}

		memcpy((void *)(ulong)dtbo_addr, (void *)((ulong)dt_img +
			be32_to_cpu(dt_entry_overlay->dt_offset)), fdt_overlay_size);

		ret = fdt_increase_size((void *)(ulong)fdt_addr, fdt_overlay_size);
		if(!ret) {
			//printf("ANDROID: fdt increase OK\n");
		} else {
			printf("ANDROID: fdt increase failed, ret=%d\n", ret);
			goto next_dtbo;
		}

		ret = fdt_overlay_apply((void *)(ulong)fdt_addr, (void *)(ulong)dtbo_addr);
		//ret = fdt_overlay_apply_verbose((void *)(ulong)fdt_addr, (void *)(ulong)dtbo_addr);
		if(!ret) {
			printf("ANDROID: apply fdt overlay %s OK\n", dtbo_token);
			_dtoverlay_bootargs_add(dev_setup);
		} else {
			printf("ANDROID: apply fdt overlay %s failed, ret=%d\n", dtbo_token, ret);
		}

next_dtbo:
		dtbo_token = strtok(NULL, " ");
	}

_exit_add_dt_overlay:
	return(ret);
}

/* Run fw_envcmd for append envtool related information in cmdline */
void _run_fw_envcmd(void)
{
	char *s;

	s = env_get("fw_envcmd");
	if (s) {
		run_command(s, 0);
	}
}
