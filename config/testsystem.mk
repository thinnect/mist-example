
# A likely channel to be used on testsystem
DEFAULT_RADIO_CHANNEL ?= 16

# Testsystem configuration limits rfpower
ifneq ($(filter tsb2,$(MAKECMDGOALS)),)
	DEFAULT_RFPOWER_DBM = -13
else
	DEFAULT_RFPOWER_DBM = -13
endif

# Don't lock testsystem builds
LOCK_BUILD ?= 0
