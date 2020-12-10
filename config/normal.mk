
# When not testsystem, use proper antenna path
ifneq ($(filter tsb2,$(MAKECMDGOALS)),)
    CFLAGS += -DDEFAULT_ANTENNA_PATH_IO2
    DEFAULT_RAIL_TX_POWER_MODE = "'M'"
    DEFAULT_RFPOWER_DBM = 10
endif
