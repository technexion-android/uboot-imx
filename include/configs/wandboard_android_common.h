/*
 * Copyright 2017 NXP
 */

#ifndef __WANDBOARD_ANDROID_COMMON_H
#define __WANDBOARD_ANDROID_COMMON_H

#define CONFIG_CI_UDC
#define CONFIG_USBD_HS
#define CONFIG_USB_GADGET_DUALSPEED

#define CONFIG_BCB_SUPPORT
#define CONFIG_CMD_READ

#define CONFIG_USB_GADGET
#define CONFIG_CMD_USB_MASS_STORAGE
#define CONFIG_USB_FUNCTION_MASS_STORAGE
#define CONFIG_USB_GADGET_DOWNLOAD
#define CONFIG_USB_GADGET_VBUS_DRAW     2

#define CONFIG_G_DNL_VENDOR_NUM         0x18d1
#define CONFIG_G_DNL_PRODUCT_NUM        0x0d02
#define CONFIG_G_DNL_MANUFACTURER	"FSL"

#define CONFIG_USB_FUNCTION_FASTBOOT
#define CONFIG_CMD_FASTBOOT
#define CONFIG_ANDROID_BOOT_IMAGE
#define CONFIG_FASTBOOT_FLASH

#define CONFIG_FSL_FASTBOOT
#define CONFIG_ANDROID_RECOVERY

#if defined CONFIG_NAND_BOOT
#define CONFIG_FASTBOOT_STORAGE_NAND
#elif defined CONFIG_SYS_BOOT_SATA
#define CONFIG_FASTBOOT_STORAGE_SATA
#else
#define CONFIG_FASTBOOT_STORAGE_MMC
#endif

#define CONFIG_CMD_BOOTA
#define CONFIG_SUPPORT_RAW_INITRD
#define CONFIG_SERIAL_TAG

#undef CONFIG_EXTRA_ENV_SETTINGS
#undef CONFIG_BOOTCOMMAND

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"bootcmd=run tn_boot_init\0" \
	"script=boot.scr\0" \
	"image=zImage\0" \
	"console=ttymxc0\0" \
	"splashpos=m,m\0" \
	"som=autodetect\0" \
	"baseboard=wandboard\0" \
	"default_baseboard=wandboard\0" \
	"fdtfile=undefined\0" \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	"bootramdisk=uramdisk.img\0" \
	"initrdaddr=0x13000000\0" \
	"fdt_addr=0x18000000\0" \
	"bootargs_base=console=ttymxc0,115200\0" \
	"boot_fdt=try\0" \
	"mmcdev=" __stringify(CONFIG_SYS_MMC_ENV_DEV) "\0" \
	"mmcpart=1\0" \
	"searchbootdev=" \
		"if test ${bootdev} = SD0; then " \
			"setenv mmcroot /dev/mmcblk2p2 rootwait rw; " \
		"else " \
			"setenv mmcroot /dev/mmcblk0p2 rootwait rw; " \
		"fi\0" \
	"mmcargs=setenv bootargs console=${console}; run videoargs\0" \
	"fdtfile_autodetect=on\0" \
	"bootdev_autodetect=on\0" \
	"display_autodetect=off\0" \
	"videoargs=" \
		"if test ${display_autodetect} = off; then " \
			"echo Applying custom display setting...;" \
			"setenv bootargs ${bootargs} ${displayinfo} ${fbmem};" \
		"else " \
			"echo Detecting monitor...;" \
			"setenv nextcon 0; " \
			"i2c dev 1; " \
			"if i2c probe 0x38; then " \
				"setenv bootargs ${bootargs} " \
					"video=mxcfb${nextcon}:dev=lcd,800x480@60," \
						"if=RGB24; " \
				"if test 0 -eq ${nextcon}; then " \
					"setenv fbmem fbmem=10M; " \
				"else " \
					"setenv fbmem ${fbmem},10M; " \
				"fi; " \
				"setexpr nextcon ${nextcon} + 1; " \
			"else " \
				"echo '- no FT5x06 touch display';" \
			"fi; " \
			"if hdmidet; then " \
				"setenv bootargs ${bootargs} " \
					"video=mxcfb${nextcon}:dev=hdmi,1280x720M@60," \
						"if=RGB24; " \
				"setenv fbmem fbmem=28M; " \
				"setexpr nextcon ${nextcon} + 1; " \
			"else " \
				"echo - no HDMI monitor;" \
			"fi; " \
			"setenv bootargs ${bootargs} ${fbmem};" \
		"fi;\0" \
	"loadbootscript=" \
		"fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${script};\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source\0" \
	"loadimage=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${image}\0" \
	"loadramdisk=fatload mmc ${mmcdev}:${mmcpart} ${initrdaddr} ${bootramdisk}\0" \
	"findfdt="\
		"if test $board_name = D1 && test $board_rev = MX6QP ; then " \
			"setenv fdtfile imx6qp-wandboard-revd1.dtb; fi; " \
		"if test $board_name = D1 && test $board_rev = MX6Q ; then " \
			"setenv fdtfile imx6q-wandboard-revd1.dtb; fi; " \
		"if test $board_name = D1 && test $board_rev = MX6DL ; then " \
			"setenv fdtfile imx6dl-wandboard-revd1.dtb; fi; " \
		"if test $board_name = C1 && test $board_rev = MX6Q ; then " \
			"setenv fdtfile imx6q-wandboard-revc1.dtb; fi; " \
		"if test $board_name = C1 && test $board_rev = MX6DL ; then " \
			"setenv fdtfile imx6dl-wandboard-revc1.dtb; fi; " \
		"if test $board_name = B1 && test $board_rev = MX6Q ; then " \
			"setenv fdtfile imx6q-wandboard-revb1.dtb; fi; " \
		"if test $board_name = B1 && test $board_rev = MX6DL ; then " \
			"setenv fdtfile imx6dl-wandboard-revb1.dtb; fi; " \
		"if test $fdtfile = undefined; then " \
			"echo WARNING: Could not determine dtb to use; fi; \0" \
	"loadfdt=fatload mmc ${mmcdev}:${mmcpart} ${fdt_addr} ${fdtfile}\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run searchbootdev; " \
		"run mmcargs; " \
		"echo baseboard is ${baseboard}; " \
		"run findfdt; " \
		"if test ${boot_fdt} = yes || test ${boot_fdt} = try; then " \
			"if run loadfdt; then " \
				"bootz ${loadaddr} - ${fdt_addr}; " \
			"else " \
				"if test ${boot_fdt} = try; then " \
					"echo WARN: Cannot load the DT; " \
					"echo fall back to load the default DT; " \
					"setenv baseboard ${default_baseboard}; " \
					"run findfdt; " \
					"run loadfdt; " \
					"bootz ${loadaddr} - ${fdt_addr}; " \
				"else " \
					"echo WARN: Cannot load the DT; " \
				"fi; " \
			"fi; " \
		"else " \
			"bootz; " \
		"fi;\0" \
	"bootenv=uEnv.txt\0" \
	"loadbootenv=fatload mmc ${mmcdev} ${loadaddr} ${bootenv}\0" \
	"importbootenv=echo Importing environment from mmc ...; " \
		"env import -t -r $loadaddr $filesize\0" \
	"tn_boot_init=" \
		   "mmc dev ${mmcdev}; if mmc rescan; then " \
		   "if run loadbootenv; then " \
			   "echo Loaded environment from ${bootenv};" \
			   "run importbootenv;" \
		   "fi;" \
		   "if test -n $uenvcmd; then " \
			   "echo Running uenvcmd ...;" \
			   "run uenvcmd;" \
		   "fi;" \
		   "if run loadbootscript; then " \
			   "run bootscript; " \
		   "fi;" \
		   "if run loadramdisk; then " \
			   "setenv rootdevice ${ramdisk_dev}; " \
			   "setenv mmcboot \'run mmcargs; bootz ${loadaddr} ${initrdaddr} ${fdt_addr}\'; " \
		   "fi;" \
		   "if run loadimage; then " \
			   "run findfdt; " \
			   "run loadfdt; " \
			   "run mmcboot; " \
			   "else " \
				   "echo WARN: Cannot load kernel from boot media; " \
			   "fi; " \
		   "fi; " \

#define CONFIG_FASTBOOT_BUF_ADDR   CONFIG_SYS_LOAD_ADDR
#define CONFIG_FASTBOOT_BUF_SIZE   0x19000000

#endif /* __WANDBOARD_ANDROID_COMMON_H */