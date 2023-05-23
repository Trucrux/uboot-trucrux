/*
 * Copyright 2017 NXP
 * Copyright 2022 Trucrux
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <cpu_func.h>
#include <hang.h>
#include <spl.h>
#include <asm/io.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/arch/ddr.h>
#include <asm/arch/imx8mq_pins.h>
#include <asm/arch/sys_proto.h>
#include <power/pmic.h>
#include <power/pfuze100_pmic.h>
#include "../../freescale/common/pfuze.h"
#include <asm/arch/clock.h>
#include <asm/mach-imx/gpio.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <fsl_esdhc_imx.h>
#include <mmc.h>
#include <power/bd71837.h>
#include "../common/imx8_eeprom.h"

DECLARE_GLOBAL_DATA_PTR;

extern struct dram_timing_info dram_timing, dram_timing_b0;

static struct trux_eeprom eeprom = {0};

static void spl_dram_init(void)
{
	trux_eeprom_read_header(&eeprom);

	if ((get_cpu_rev() & 0xfff) == CHIP_REV_2_1) {
		trux_eeprom_adjust_dram(&eeprom, &dram_timing);
		ddr_init(&dram_timing);
	}
	else {
		trux_eeprom_adjust_dram(&eeprom, &dram_timing_b0);
		ddr_init(&dram_timing_b0);
	}
}

#define I2C_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_HYS | PAD_CTL_PUE)
#define PC MUX_PAD_CTRL(I2C_PAD_CTRL)

struct i2c_pads_info i2c_pad_info1 = {
	.scl = {
		.i2c_mode = IMX8MQ_PAD_I2C1_SCL__I2C1_SCL | PC,
		.gpio_mode = IMX8MQ_PAD_I2C1_SCL__GPIO5_IO14 | PC,
		.gp = IMX_GPIO_NR(5, 14),
	},
	.sda = {
		.i2c_mode = IMX8MQ_PAD_I2C1_SDA__I2C1_SDA | PC,
		.gpio_mode = IMX8MQ_PAD_I2C1_SDA__GPIO5_IO15 | PC,
		.gp = IMX_GPIO_NR(5, 15),
	},
};

struct i2c_pads_info i2c_pad_info3 = {
	.scl = {
		.i2c_mode = IMX8MQ_PAD_I2C3_SCL__I2C3_SCL | PC,
		.gpio_mode = IMX8MQ_PAD_I2C3_SCL__GPIO5_IO18 | PC,
		.gp = IMX_GPIO_NR(5, 18),
	},
	.sda = {
		.i2c_mode = IMX8MQ_PAD_I2C3_SDA__I2C3_SDA | PC,
		.gpio_mode = IMX8MQ_PAD_I2C3_SDA__GPIO5_IO19 | PC,
		.gp = IMX_GPIO_NR(5, 19),
	},
};

#define USDHC1_PWR_GPIO IMX_GPIO_NR(2, 10)
#define USDHC2_PWR_GPIO IMX_GPIO_NR(2, 19)

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	switch (cfg->esdhc_base) {
	case USDHC1_BASE_ADDR:
		ret = 1;
		break;
	case USDHC2_BASE_ADDR:
		ret = 1;
		return ret;
	}

	return 1;
}

#define USDHC_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_HYS | PAD_CTL_PUE | \
			 PAD_CTL_FSEL2)
#define USDHC_GPIO_PAD_CTRL (PAD_CTL_PUE | PAD_CTL_DSE1)

static iomux_v3_cfg_t const usdhc1_pads[] = {
	IMX8MQ_PAD_SD1_CLK__USDHC1_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_CMD__USDHC1_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_DATA0__USDHC1_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_DATA1__USDHC1_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_DATA2__USDHC1_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_DATA3__USDHC1_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_DATA4__USDHC1_DATA4 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_DATA5__USDHC1_DATA5 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_DATA6__USDHC1_DATA6 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_DATA7__USDHC1_DATA7 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_RESET_B__GPIO2_IO10 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static iomux_v3_cfg_t const usdhc2_pads[] = {
	IMX8MQ_PAD_SD2_CLK__USDHC2_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL), /* 0xd6 */
	IMX8MQ_PAD_SD2_CMD__USDHC2_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL), /* 0xd6 */
	IMX8MQ_PAD_SD2_DATA0__USDHC2_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL), /* 0xd6 */
	IMX8MQ_PAD_SD2_DATA1__USDHC2_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL), /* 0xd6 */
	IMX8MQ_PAD_SD2_DATA2__USDHC2_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL), /* 0x16 */
	IMX8MQ_PAD_SD2_DATA3__USDHC2_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL), /* 0xd6 */
	IMX8MQ_PAD_SD2_RESET_B__GPIO2_IO19 | MUX_PAD_CTRL(USDHC_GPIO_PAD_CTRL),
};

static struct fsl_esdhc_cfg usdhc_cfg[2] = {
	{USDHC1_BASE_ADDR, 0, 8},
	{USDHC2_BASE_ADDR, 0, 4},
};

static const char bd71837_name[] = "BD71837";
int power_bd71837_init (unsigned char bus) {
		struct pmic *p = pmic_alloc();
		if (!p) {
				printf("%s: POWER allocation error!\n", __func__);
				return -ENOMEM;
		}

		p->name = bd71837_name;
		p->interface = PMIC_I2C;
		p->number_of_regs = BD71837_REG_NUM;
		p->hw.i2c.addr = 0x4b;
		p->hw.i2c.tx_num = 1;
		p->bus = bus;

		debug("power_bd71837_init\n");

		return 0;
}

int board_mmc_init(bd_t *bis)
{
	int i, ret;
	/*
	 * According to the board_mmc_init() the following map is done:
	 * (U-Boot device node)    (Physical Port)
	 * mmc0                    USDHC1
	 * mmc1                    USDHC2
	 */
	for (i = 0; i < CONFIG_SYS_FSL_USDHC_NUM; i++) {
		switch (i) {
		case 0:
			init_clk_usdhc(0);
			usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
			imx_iomux_v3_setup_multiple_pads(usdhc1_pads,
							 ARRAY_SIZE(usdhc1_pads));
			gpio_request(USDHC1_PWR_GPIO, "usdhc1_reset");
			gpio_direction_output(USDHC1_PWR_GPIO, 0);
			udelay(500);
			gpio_direction_output(USDHC1_PWR_GPIO, 1);
			break;
		case 1:
			init_clk_usdhc(1);
			usdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
			imx_iomux_v3_setup_multiple_pads(usdhc2_pads,
							 ARRAY_SIZE(usdhc2_pads));
			gpio_request(USDHC2_PWR_GPIO, "usdhc2_reset");
			gpio_direction_output(USDHC2_PWR_GPIO, 0);
			udelay(500);
			gpio_direction_output(USDHC2_PWR_GPIO, 1);
			break;
		default:
			printf("Warning: you configured more USDHC controllers"
				"(%d) than supported by the board\n", i + 1);
			return -EINVAL;
		}

		ret = fsl_esdhc_initialize(bis, &usdhc_cfg[i]);
		if (ret)
			return ret;
	}

	return 0;
}

#ifdef CONFIG_POWER

#define PMIC_I2C_BUS		0
#define VDD_ARM_I2C_BUS		0
#define VDD_SOC_I2C_BUS		2

#define VDD_I2C_ADDR	0x60

/* VDD Regulator Registers */
#define VDD_SET0	0x00
#define VDD_CTRL	0x04
#define VDD_TEMP	0x05
#define VDD_RMPCTRL	0x06
#define VDD_CHIP_ID1	0x08
#define VDD_CHIP_ID2	0x09

int set_vdd_regulator(int bus, char *name)
{
	uint8_t val1, val2;

	/* Probe VDD regulator */
		if (!((i2c_set_bus_num(bus) == 0) &&
			  (i2c_probe(VDD_I2C_ADDR) == 0))) {
		printf("%s: i2c bus failed\n", name);
		return -1;
		}

	/* Read regulator chip ID */
	if (!((i2c_read(VDD_I2C_ADDR, VDD_CHIP_ID1, 1, &val1, 1) == 0) &&
		  (i2c_read(VDD_I2C_ADDR, VDD_CHIP_ID2, 1, &val2, 1) == 0) &&
		  (val1 == val2))) {
		printf("%s: chip ID read failed\n", name);
		return -1;
		}

		debug("%s: Chip ID: 0x%.2x\n", name, val1);

	/* Reset temperature alarm */
	val1 = 0x0;
	i2c_write(VDD_I2C_ADDR, VDD_TEMP, 1, &val1, 1);

	/* Set Voltage:0.9V + MODE:PWM */
	val1 = 0x28;
	i2c_write(VDD_I2C_ADDR, VDD_SET0, 1, &val1, 1);

	return 0;
}


int power_init_board(void)
{
	struct pmic *p;
		int ret;
		ret = power_bd71837_init(PMIC_I2C_BUS);
		if (ret)
			printf("power init failed\n");
		else
			printf("PMIC: BD71837 Found\n");

		p = pmic_get("BD71837");
		pmic_probe(p);

		/* decrease RESET key long push time from the default 10s to 10ms */
		pmic_reg_write(p, BD71837_PWRONCONFIG1, 0x0);

		/* unlock the PMIC regs */
		pmic_reg_write(p, BD71837_REGLOCK, 0x1);

		/* increase VDD_SOC to typical value 0.85v before first DRAM access */
		pmic_reg_write(p, BD71837_BUCK1_VOLT_RUN, 0x0f);

		/* increase VDD_DRAM to 0.975v for 3Ghz DDR */
		pmic_reg_write(p, BD71837_BUCK5_VOLT, 0x83);

		/* Enabled NVCC_DRAM */
		pmic_reg_write(p, BD71837_BUCK8_CTRL, 0x03);

		/* Enable LDO5 - PHY supply to 1.8V */
		pmic_reg_write(p, BD71837_LDO5_VOLT, 0xc0);

		/* lock the PMIC regs */
		pmic_reg_write(p, BD71837_REGLOCK, 0x11);

		return 0;

}
#endif

void spl_board_init(void)
{
	struct trux_eeprom *ep = TRUX_EEPROM_DATA;
#ifndef CONFIG_SPL_USB_SDP_SUPPORT
	/* Serial download mode */
	if (is_usb_boot()) {
		puts("Back to ROM, SDP\n");
		restore_boot_params();
	}
#endif

	init_usb_clk();

	puts("Normal Boot\n");

	/* Copy EEPROM contents to DRAM */
	memcpy(ep, &eeprom, sizeof(*ep));
}

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	/* Just empty function now - can't decide what to choose */
	debug("%s: %s\n", __func__, name);

	return 0;
}
#endif

void board_init_f(ulong dummy)
{
	int ret;

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	arch_cpu_init();

	board_early_init_f();

	timer_init();

	preloader_console_init();

	ret = spl_init();
	if (ret) {
		debug("spl_init() failed: %d\n", ret);
		hang();
	}

	enable_tzc380();

	/* Adjust pmic voltage to 1.0V for 800M */
	setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info1);
	setup_i2c(2, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info3);

	power_init_board();

	/* DDR initialization */
	spl_dram_init();

	board_init_r(NULL, 0);
}

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	puts ("resetting ...\n");

	reset_cpu(WDOG1_BASE_ADDR);

	return 0;
}
