/*
 * Copyright Trucrux.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#ifndef _MX8_TRUX_EEPROM_H_
#define _MX8_TRUX_EEPROM_H_

#ifdef CONFIG_ARCH_IMX8M
#include <asm/arch-imx8m/ddr.h>
#endif

#define TRUX_EEPROM_MAGIC	0x384D /* == HEX("8M") */

#define TRUX_EEPROM_I2C_BUS	0
#define TRUX_EEPROM_I2C_ADDR	0x52

/* Optional SOM features */
#define TRUX_EEPROM_F_WIFI 	(1 << 0)
#define TRUX_EEPROM_F_ETH 	(1 << 1)
#define TRUX_EEPROM_F_AUDIO 	(1 << 2)
#define TRUX_EEPROM_F_MX8M_LVDS	(1 << 3) /* i.MX8MM, i.MX8MN, i.MX8MQ only */
#define TRUX_EEPROM_F_MX8Q_SOC_ID (1 << 3) /* 0 = i.MX8QM, 1 = i.MX8QP */
#define TRUX_EEPROM_F_NAND	(1 << 4)

/* SOM storage types */
enum som_storage {
	SOM_STORAGE_EMMC,
	SOM_STORAGE_NAND,
	SOM_STORAGE_UNDEFINED,
};

/* Number of DRAM adjustment tables */
#define DRAM_TABLE_NUM 7

struct __attribute__((packed)) trux_eeprom
{
	u16 magic;                /* 00-0x00 - magic number       */
	u8 partnum[3];            /* 02-0x02 - part number        */
	u8 assembly[10];          /* 05-0x05 - assembly number    */
	u8 date[9];               /* 15-0x0f - build date         */
	u8 mac[6];                /* 24-0x18 - MAC address        */
	u8 somrev;                /* 30-0x1e - SOM revision       */
	u8 version;               /* 31-0x1f - EEPROM version     */
	u8 features;              /* 32-0x20 - SOM features       */
	u8 dramsize;              /* 33-0x21 - DRAM size          */
	u8 off[DRAM_TABLE_NUM+1]; /* 34-0x22 - DRAM table offsets */
	u8 partnum2[5];           /* 42-0x2a - part number        */
	u8 reserved[3];           /* 47 0x2f - reserved           */
};

#define TRUX_EEPROM_DATA ((struct trux_eeprom *)TRUX_EEPROM_DRAM_START)

#define TRUX_CARRIER_EEPROM_MAGIC	0x5643 /* == HEX("VC") */

struct __attribute__((packed)) trux_carrier_eeprom
{
	u16 magic;		/* 00-0x00 - magic number		*/
	u8 struct_ver;		/* 01-0x01 - EEPROM structure version	*/
	u8 carrier_rev[16];	/* 02-0x02 - carrier board revision	*/
	u32 crc;		/* 10-0x0a - checksum			*/
};

static inline int trux_eeprom_is_valid(struct trux_eeprom *ep)
{
	if (htons(ep->magic) != TRUX_EEPROM_MAGIC) {
		debug("Invalid EEPROM magic 0x%hx, expected 0x%hx\n",
			htons(ep->magic), TRUX_EEPROM_MAGIC);
		return 0;
	}

	return 1;
}

int trux_eeprom_read_header(struct trux_eeprom *e);
int trux_scu_eeprom_read_header(struct trux_eeprom *e);
int trux_eeprom_get_dram_size(struct trux_eeprom *e, phys_size_t *size);
int trux_eeprom_get_mac(struct trux_eeprom *e, u8 *mac);
int trux_eeprom_get_storage(struct trux_eeprom *e, int *storage);
void trux_eeprom_print_prod_info(struct trux_eeprom *e);

#if defined(CONFIG_ARCH_IMX8M) && defined(CONFIG_SPL_BUILD)
void trux_eeprom_adjust_dram(struct trux_eeprom *e, struct dram_timing_info *d);
#endif

int trux_carrier_eeprom_read(int bus, int addr, struct trux_carrier_eeprom *ep);
int trux_carrier_eeprom_is_valid(struct trux_carrier_eeprom *ep);
void trux_carrier_eeprom_get_revision(struct trux_carrier_eeprom *ep, char *rev, size_t size);

#endif /* _MX8M_TRUX_EEPROM_H_ */
