/*
 * Copyright (C) 2015-2016 Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/imx-common/boot_mode.h>
#include <asm/imx-common/dma.h>
#include <asm/imx-common/hab.h>
#include <asm/imx-common/rdc-sema.h>
#include <asm/arch/imx-rdc.h>
#include <asm/arch/crm_regs.h>
#include <dm.h>
#include <fdt_support.h>
#include <imx_thermal.h>
#include <fsl_wdog.h>
#if defined(CONFIG_FSL_FASTBOOT) && defined(CONFIG_ANDROID_RECOVERY)
#include <recovery.h>
#endif

#if defined(CONFIG_IMX_THERMAL)
static const struct imx_thermal_plat imx7_thermal_plat = {
	.regs = (void *)ANATOP_BASE_ADDR,
	.fuse_bank = 3,
	.fuse_word = 3,
};

U_BOOT_DEVICE(imx7_thermal) = {
	.name = "imx_thermal",
	.platdata = &imx7_thermal_plat,
};
#endif

#ifdef CONFIG_IMX_RDC
/*
 * In current design, if any peripheral was assigned to both A7 and M4,
 * it will receive ipg_stop or ipg_wait when any of the 2 platforms enter
 * low power mode. So M4 sleep will cause some peripherals fail to work
 * at A7 core side. At default, all resources are in domain 0 - 3.
 *
 * There are 26 peripherals impacted by this IC issue:
 * SIM2(sim2/emvsim2)
 * SIM1(sim1/emvsim1)
 * UART1/UART2/UART3/UART4/UART5/UART6/UART7
 * SAI1/SAI2/SAI3
 * WDOG1/WDOG2/WDOG3/WDOG4
 * GPT1/GPT2/GPT3/GPT4
 * PWM1/PWM2/PWM3/PWM4
 * ENET1/ENET2
 * Software Workaround:
 * Here we setup some resources to domain 0 where M4 codes will move
 * the M4 out of this domain. Then M4 is not able to access them any longer.
 * This is a workaround for ic issue. So the peripherals are not shared
 * by them. This way requires the uboot implemented the RDC driver and
 * set the 26 IPs above to domain 0 only. M4 code will assign resource
 * to its own domain, if it want to use the resource.
 */
static rdc_peri_cfg_t const resources[] = {
	(RDC_PER_SIM1 | RDC_DOMAIN(0)),
	(RDC_PER_SIM2 | RDC_DOMAIN(0)),
	(RDC_PER_UART1 | RDC_DOMAIN(0)),
	(RDC_PER_UART2 | RDC_DOMAIN(0)),
	(RDC_PER_UART3 | RDC_DOMAIN(0)),
	(RDC_PER_UART4 | RDC_DOMAIN(0)),
	(RDC_PER_UART5 | RDC_DOMAIN(0)),
	(RDC_PER_UART6 | RDC_DOMAIN(0)),
	(RDC_PER_UART7 | RDC_DOMAIN(0)),
	(RDC_PER_SAI1 | RDC_DOMAIN(0)),
	(RDC_PER_SAI2 | RDC_DOMAIN(0)),
	(RDC_PER_SAI3 | RDC_DOMAIN(0)),
	(RDC_PER_WDOG1 | RDC_DOMAIN(0)),
	(RDC_PER_WDOG2 | RDC_DOMAIN(0)),
	(RDC_PER_WDOG3 | RDC_DOMAIN(0)),
	(RDC_PER_WDOG4 | RDC_DOMAIN(0)),
	(RDC_PER_GPT1 | RDC_DOMAIN(0)),
	(RDC_PER_GPT2 | RDC_DOMAIN(0)),
	(RDC_PER_GPT3 | RDC_DOMAIN(0)),
	(RDC_PER_GPT4 | RDC_DOMAIN(0)),
	(RDC_PER_PWM1 | RDC_DOMAIN(0)),
	(RDC_PER_PWM2 | RDC_DOMAIN(0)),
	(RDC_PER_PWM3 | RDC_DOMAIN(0)),
	(RDC_PER_PWM4 | RDC_DOMAIN(0)),
	(RDC_PER_ENET1 | RDC_DOMAIN(0)),
	(RDC_PER_ENET2 | RDC_DOMAIN(0)),
};

static void isolate_resource(void)
{
	imx_rdc_setup_peripherals(resources, ARRAY_SIZE(resources));
}
#endif

#if defined(CONFIG_SECURE_BOOT)
struct imx_sec_config_fuse_t const imx_sec_config_fuse = {
	.bank = 1,
	.word = 3,
};
#endif

/*
 * OCOTP_TESTER3[9:8] (see Fusemap Description Table offset 0x440)
 * defines a 2-bit SPEED_GRADING
 */
#define OCOTP_TESTER3_SPEED_SHIFT	8
#define OCOTP_TESTER3_SPEED_800MHZ	0
#define OCOTP_TESTER3_SPEED_850MHZ	1
#define OCOTP_TESTER3_SPEED_1GHZ	2
#define OCOTP_TESTER3_SPEED_1P2GHZ	3

u32 get_cpu_speed_grade_hz(void)
{
	struct ocotp_regs *ocotp = (struct ocotp_regs *)OCOTP_BASE_ADDR;
	struct fuse_bank *bank = &ocotp->bank[1];
	struct fuse_bank1_regs *fuse =
		(struct fuse_bank1_regs *)bank->fuse_regs;
	uint32_t val;

	val = readl(&fuse->tester3);
	val >>= OCOTP_TESTER3_SPEED_SHIFT;
	val &= 0x3;

	switch(val) {
	case OCOTP_TESTER3_SPEED_800MHZ:
		return 792000000;
	case OCOTP_TESTER3_SPEED_850MHZ:
		return 852000000;
	case OCOTP_TESTER3_SPEED_1GHZ:
		return 996000000;
	case OCOTP_TESTER3_SPEED_1P2GHZ:
		return 1200000000;
	}
	return 0;
}

/*
 * OCOTP_TESTER3[7:6] (see Fusemap Description Table offset 0x440)
 * defines a 2-bit SPEED_GRADING
 */
#define OCOTP_TESTER3_TEMP_SHIFT	6

u32 get_cpu_temp_grade(int *minc, int *maxc)
{
	struct ocotp_regs *ocotp = (struct ocotp_regs *)OCOTP_BASE_ADDR;
	struct fuse_bank *bank = &ocotp->bank[1];
	struct fuse_bank1_regs *fuse =
		(struct fuse_bank1_regs *)bank->fuse_regs;
	uint32_t val;

	val = readl(&fuse->tester3);
	val >>= OCOTP_TESTER3_TEMP_SHIFT;
	val &= 0x3;

	if (minc && maxc) {
		if (val == TEMP_AUTOMOTIVE) {
			*minc = -40;
			*maxc = 125;
		} else if (val == TEMP_INDUSTRIAL) {
			*minc = -40;
			*maxc = 105;
		} else if (val == TEMP_EXTCOMMERCIAL) {
			*minc = -20;
			*maxc = 105;
		} else {
			*minc = 0;
			*maxc = 95;
		}
	}
	return val;
}

static bool is_mx7d(void)
{
	struct ocotp_regs *ocotp = (struct ocotp_regs *)OCOTP_BASE_ADDR;
	struct fuse_bank *bank = &ocotp->bank[1];
	struct fuse_bank1_regs *fuse =
		(struct fuse_bank1_regs *)bank->fuse_regs;
	int val;

	val = readl(&fuse->tester4);
	if (val & 1)
		return false;
	else
		return true;
}

u32 get_cpu_rev(void)
{
	struct mxc_ccm_anatop_reg *ccm_anatop = (struct mxc_ccm_anatop_reg *)
						 ANATOP_BASE_ADDR;
	u32 reg = readl(&ccm_anatop->digprog);
	u32 type = (reg >> 16) & 0xff;

	if (!is_mx7d())
		type = MXC_CPU_MX7S;

	reg &= 0xff;
	return (type << 12) | reg;
}

#ifdef CONFIG_REVISION_TAG
u32 __weak get_board_rev(void)
{
	return get_cpu_rev();
}
#endif

/* enable all periherial can be accessed in nosec mode */
static void init_csu(void)
{
	int i = 0;
	for (i = 0; i < CSU_NUM_REGS; i++)
		writel(CSU_INIT_SEC_LEVEL0, CSU_IPS_BASE_ADDR + i * 4);
}

static void imx_enet_mdio_fixup(void)
{
	struct iomuxc_gpr_base_regs *gpr_regs =
		(struct iomuxc_gpr_base_regs *)IOMUXC_GPR_BASE_ADDR;

	/*
	 * The management data input/output (MDIO) requires open-drain,
	 * i.MX7D TO1.0 ENET MDIO pin has no open drain, but TO1.1 supports
	 * this feature. So to TO1.1, need to enable open drain by setting
	 * bits GPR0[8:7].
	 */

	if (soc_rev() >= CHIP_REV_1_1) {
		setbits_le32(&gpr_regs->gpr[0],
			     IOMUXC_GPR_GPR0_ENET_MDIO_OPEN_DRAIN_MASK);
	}
}

static void set_epdc_qos(void)
{
#define REGS_QOS_BASE     QOSC_IPS_BASE_ADDR
#define REGS_QOS_EPDC     (QOSC_IPS_BASE_ADDR + 0x3400)
#define REGS_QOS_PXP0     (QOSC_IPS_BASE_ADDR + 0x2C00)
#define REGS_QOS_PXP1     (QOSC_IPS_BASE_ADDR + 0x3C00)

	writel(0, REGS_QOS_BASE);  /*  Disable clkgate & soft_reset */
	writel(0, REGS_QOS_BASE + 0x60);  /*  Enable all masters */
	writel(0, REGS_QOS_EPDC);   /*  Disable clkgate & soft_reset */
	writel(0, REGS_QOS_PXP0);   /*  Disable clkgate & soft_reset */
	writel(0, REGS_QOS_PXP1);   /*  Disable clkgate & soft_reset */

	writel(0x0f020722, REGS_QOS_EPDC + 0xd0);   /*  WR, init = 7 with red flag */
	writel(0x0f020722, REGS_QOS_EPDC + 0xe0);   /*  RD,  init = 7 with red flag */

	writel(1, REGS_QOS_PXP0);   /*  OT_CTRL_EN =1 */
	writel(1, REGS_QOS_PXP1);   /*  OT_CTRL_EN =1 */

	writel(0x0f020222, REGS_QOS_PXP0 + 0x50);   /*  WR,  init = 2 with red flag */
	writel(0x0f020222, REGS_QOS_PXP1 + 0x50);   /*  WR,  init = 2 with red flag */
	writel(0x0f020222, REGS_QOS_PXP0 + 0x60);   /*  rD,  init = 2 with red flag */
	writel(0x0f020222, REGS_QOS_PXP1 + 0x60);   /*  rD,  init = 2 with red flag */
	writel(0x0f020422, REGS_QOS_PXP0 + 0x70);   /*  tOTAL,  init = 4 with red flag */
	writel(0x0f020422, REGS_QOS_PXP1 + 0x70);   /*  TOTAL,  init = 4 with red flag */

	writel(0xe080, IOMUXC_GPR_BASE_ADDR + 0x0034); /* EPDC AW/AR CACHE ENABLE */
}

int arch_cpu_init(void)
{
	init_aips();

	init_csu();
	/* Disable PDE bit of WMCR register */
	imx_set_wdog_powerdown(false);

	imx_enet_mdio_fixup();

	set_epdc_qos();

#ifdef CONFIG_APBH_DMA
	/* Start APBH DMA */
	mxs_dma_init();
#endif

#ifdef CONFIG_IMX_RDC
	isolate_resource();
#endif

	return 0;
}

#ifdef CONFIG_ARCH_MISC_INIT
int arch_misc_init(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	if (is_mx7d())
		setenv("soc", "imx7d");
	else
		setenv("soc", "imx7s");
#endif

	return 0;
}
#endif

#ifdef CONFIG_SERIAL_TAG
void get_board_serial(struct tag_serialnr *serialnr)
{
	struct ocotp_regs *ocotp = (struct ocotp_regs *)OCOTP_BASE_ADDR;
	struct fuse_bank *bank = &ocotp->bank[0];
	struct fuse_bank0_regs *fuse =
		(struct fuse_bank0_regs *)bank->fuse_regs;

	serialnr->low = fuse->tester0;
	serialnr->high = fuse->tester1;
}
#endif

#if defined(CONFIG_FEC_MXC)
void imx_get_mac_from_fuse(int dev_id, unsigned char *mac)
{
	struct ocotp_regs *ocotp = (struct ocotp_regs *)OCOTP_BASE_ADDR;
	struct fuse_bank *bank = &ocotp->bank[9];
	struct fuse_bank9_regs *fuse =
		(struct fuse_bank9_regs *)bank->fuse_regs;

	if (0 == dev_id) {
		u32 value = readl(&fuse->mac_addr1);
		mac[0] = (value >> 8);
		mac[1] = value;

		value = readl(&fuse->mac_addr0);
		mac[2] = value >> 24;
		mac[3] = value >> 16;
		mac[4] = value >> 8;
		mac[5] = value;
	} else {
		u32 value = readl(&fuse->mac_addr2);
		mac[0] = value >> 24;
		mac[1] = value >> 16;
		mac[2] = value >> 8;
		mac[3] = value;

		value = readl(&fuse->mac_addr1);
		mac[4] = value >> 24;
		mac[5] = value >> 16;
	}
}
#endif

#ifdef CONFIG_IMX_BOOTAUX
int arch_auxiliary_core_up(u32 core_id, u32 boot_private_data)
{
	u32 stack, pc;
	struct src *src_reg = (struct src *)SRC_BASE_ADDR;

	if (!boot_private_data)
		return 1;

	stack = *(u32 *)boot_private_data;
	pc = *(u32 *)(boot_private_data + 4);

	/* Set the stack and pc to M4 bootROM */
	writel(stack, M4_BOOTROM_BASE_ADDR);
	writel(pc, M4_BOOTROM_BASE_ADDR + 4);

	/* Enable M4 */
	clrsetbits_le32(&src_reg->m4rcr, SRC_M4RCR_M4C_NON_SCLR_RST_MASK,
			SRC_M4RCR_ENABLE_M4_MASK);

	return 0;
}

int arch_auxiliary_core_check_up(u32 core_id)
{
	uint32_t val;
	struct src *src_reg = (struct src *)SRC_BASE_ADDR;

	val = readl(&src_reg->m4rcr);
	if (val & 0x00000001)
		return 0; /* assert in reset */

	return 1;
}
#endif

void set_wdog_reset(struct wdog_regs *wdog)
{
	u32 reg = readw(&wdog->wcr);
	/*
	 * Output WDOG_B signal to reset external pmic or POR_B decided by
	 * the board desgin. Without external reset, the peripherals/DDR/
	 * PMIC are not reset, that may cause system working abnormal.
	 */
	reg = readw(&wdog->wcr);
	reg |= 1 << 3;
	/*
	 * WDZST bit is write-once only bit. Align this bit in kernel,
	 * otherwise kernel code will have no chance to set this bit.
	 */
	reg |= 1 << 0;
	writew(reg, &wdog->wcr);
}

/*
 * cfg_val will be used for
 * Boot_cfg4[7:0]:Boot_cfg3[7:0]:Boot_cfg2[7:0]:Boot_cfg1[7:0]
 * After reset, if GPR10[28] is 1, ROM will copy GPR9[25:0]
 * to SBMR1, which will determine the boot device.
 */
const struct boot_mode soc_boot_modes[] = {
	{"ecspi1:0",	MAKE_CFGVAL(0x00, 0x60, 0x00, 0x00)},
	{"ecspi1:1",	MAKE_CFGVAL(0x40, 0x62, 0x00, 0x00)},
	{"ecspi1:2",	MAKE_CFGVAL(0x80, 0x64, 0x00, 0x00)},
	{"ecspi1:3",	MAKE_CFGVAL(0xc0, 0x66, 0x00, 0x00)},

	{"weim",	MAKE_CFGVAL(0x00, 0x50, 0x00, 0x00)},
	{"qspi1",	MAKE_CFGVAL(0x10, 0x40, 0x00, 0x00)},
	/* 4 bit bus width */
	{"usdhc1",	MAKE_CFGVAL(0x10, 0x10, 0x00, 0x00)},
	{"usdhc2",	MAKE_CFGVAL(0x10, 0x14, 0x00, 0x00)},
	{"usdhc3",	MAKE_CFGVAL(0x10, 0x18, 0x00, 0x00)},
	{"mmc1",	MAKE_CFGVAL(0x10, 0x20, 0x00, 0x00)},
	{"mmc2",	MAKE_CFGVAL(0x10, 0x24, 0x00, 0x00)},
	{"mmc3",	MAKE_CFGVAL(0x10, 0x28, 0x00, 0x00)},
	{NULL,		0},
};

enum boot_device get_boot_device(void)
{
	struct bootrom_sw_info **p =
		(struct bootrom_sw_info **)ROM_SW_INFO_ADDR;

	enum boot_device boot_dev = SD1_BOOT;
	u8 boot_type = (*p)->boot_dev_type;
	u8 boot_instance = (*p)->boot_dev_instance;

	switch (boot_type) {
	case BOOT_TYPE_SD:
		boot_dev = boot_instance + SD1_BOOT;
		break;
	case BOOT_TYPE_MMC:
		boot_dev = boot_instance + MMC1_BOOT;
		break;
	case BOOT_TYPE_NAND:
		boot_dev = NAND_BOOT;
		break;
	case BOOT_TYPE_QSPI:
		boot_dev = QSPI_BOOT;
		break;
	case BOOT_TYPE_WEIM:
		boot_dev = WEIM_NOR_BOOT;
		break;
	case BOOT_TYPE_SPINOR:
		boot_dev = SPI_NOR_BOOT;
		break;
	default:
		break;
	}

	return boot_dev;
}

#ifdef CONFIG_ENV_IS_IN_MMC
__weak int board_mmc_get_env_dev(int devno)
{
	return CONFIG_SYS_MMC_ENV_DEV;
}

int mmc_get_env_dev(void)
{
	struct bootrom_sw_info **p =
		(struct bootrom_sw_info **)ROM_SW_INFO_ADDR;
	int devno = (*p)->boot_dev_instance;
	u8 boot_type = (*p)->boot_dev_type;

	/* If not boot from sd/mmc, use default value */
	if ((boot_type != BOOT_TYPE_SD) && (boot_type != BOOT_TYPE_MMC))
		return CONFIG_SYS_MMC_ENV_DEV;

	return board_mmc_get_env_dev(devno);
}
#endif

void s_init(void)
{
	/* clock configuration. */
	clock_init();

#if defined(CONFIG_ANDROID_SUPPORT)
        /* Enable RTC */
        writel(0x21, 0x30370038);
#endif
	return;
}

void reset_misc(void)
{
#ifdef CONFIG_VIDEO_MXS
	lcdif_power_down();
#endif
}

void reset_cpu(ulong addr)
{
	struct watchdog_regs *wdog = (struct watchdog_regs *)WDOG1_BASE_ADDR;

	/* Clear WDA to trigger WDOG_B immediately */
	writew((WCR_WDE | WCR_SRS), &wdog->wcr);

	while (1) {
		/*
		 * spin for .5 seconds before reset
		 */
	}
}


#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
/*
 * fdt_pack_reg - pack address and size array into the "reg"-suitable stream
 */
static int fdt_pack_reg(const void *fdt, void *buf, u64 *address, u64 *size,
                        int n)
{
        int i;
        int address_cells = fdt_address_cells(fdt, 0);
        int size_cells = fdt_size_cells(fdt, 0);
        char *p = buf;

        for (i = 0; i < n; i++) {
                if (address_cells == 2)
                        *(fdt64_t *)p = cpu_to_fdt64(address[i]);
                else
                        *(fdt32_t *)p = cpu_to_fdt32(address[i]);
                p += 4 * address_cells;

                if (size_cells == 2)
                        *(fdt64_t *)p = cpu_to_fdt64(size[i]);
                else
                        *(fdt32_t *)p = cpu_to_fdt32(size[i]);
                p += 4 * size_cells;
        }

        return p - (char *)buf;
}

#ifdef CONFIG_NR_DRAM_BANKS
#define MEMORY_BANKS_MAX CONFIG_NR_DRAM_BANKS
#else
#define MEMORY_BANKS_MAX 4
#endif
int ft_board_setup(void *blob, bd_t *bd)
{
	__maybe_unused u32 max_freq_khz;
	int err, i;
	int nodeoffset, plen, len = 0;
	u32 *prop, newprop[12];

	err = fdt_check_header(blob);
	if (err < 0) {
		printf("%s: %s\n", __FUNCTION__, fdt_strerror(err));
		return err;
	}

	nodeoffset = fdt_path_offset(blob, "/cpus/cpu@0");
	if (nodeoffset >= 0) {
		prop = (u32 *)fdt_getprop(blob, nodeoffset,
					    "operating-points", &plen);
		if (prop) {
			max_freq_khz = get_cpu_speed_grade_hz() / 1000;

			for(i=0; i<plen; i+=8) {
				if(__be32_to_cpu(*prop) <= max_freq_khz) {
					newprop[len] = *prop++;
					newprop[len + 1] = *prop++;
					len += 2;
				}else
					prop += 2;
			}

			err = fdt_delprop(blob, nodeoffset,
					  "operating-points");
			if (err < 0)
				printf("error deleting operating-points\n");

			err = fdt_setprop(blob, nodeoffset,
					  "operating-points",
					  newprop,
					  (len * sizeof(u32)));
			if (err < 0)
				printf("error adding operating-points\n");
		}
	}

#if defined(CONFIG_IMX_BOOTAUX)
	if(arch_auxiliary_core_check_up(0)) {
		int banks;
		u64 start[2], size[2];
		u8 tmp[MEMORY_BANKS_MAX * 32]; /* Up to 64-bit address + 64-bit size */

		/* find or create "/memory" node. */
		nodeoffset = fdt_find_or_add_subnode(blob, 0, "memory");
		if (nodeoffset < 0)
			return nodeoffset;

		start[0] = bd->bi_dram[0].start;
		size[0] = SZ_512M - SZ_1M;
		banks = 1;
		if (bd->bi_dram[0].size > SZ_512M) {
			start[1] = bd->bi_dram[0].start + SZ_512M;
			size[1] = bd->bi_dram[0].size - SZ_512M;
			banks = 2;
		}

		len = fdt_pack_reg(blob, tmp, start, size, banks);
		err = fdt_setprop(blob, nodeoffset, "linux,usable-memory", tmp, len);
		if (err < 0) {
				printf("WARNING: could not set %s %s.\n",
								"linux,usable-memory", fdt_strerror(err));
				return err;
		}
	} else {
		nodeoffset = fdt_node_offset_by_compatible(blob, -1, "fsl,imx7d-rpmsg");
		if (nodeoffset > 0)
			fdt_set_node_status(blob, nodeoffset, FDT_STATUS_DISABLED, 0);
	}
#endif
	return 0;
}
#endif