int trux_get_som_rev(struct trux_eeprom *ep);
int get_board_id(void);

enum {
	TRUX_MX8M_MINI,
	UNKNOWN_BOARD,
};

enum {
	SOM_REV_10,
	SOM_REV_11,
	SOM_REV_12,
	SOM_REV_13,
	UNKNOWN_REV
};

enum {
	TRUX_CARRIER_REV_1,
	TRUX_CARRIER_REV_2,
	TRUX_CARRIER_REV_UNDEF,
};
