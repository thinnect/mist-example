# This is the main makefile for MistExample firmware

# _______________________ User overridable configuration _______________________

PROJECT_NAME            ?= mistexample

VERSION_MAJOR           ?= 1
VERSION_MINOR           ?= 0
VERSION_PATCH           ?= 0
VERSION_DEVEL           ?= "-dev"

# Include some optional configuration when requested
CONFIG ?= normal
$(info CONFIG=$(CONFIG))
include config/$(CONFIG).mk

DEFAULT_RADIO_CHANNEL   ?= 13

# Set device address at compile time, will override signature when != 0
NODE_AM_ADDR            ?= 0
DEFAULT_PAN_ID          ?= 0x22

#include bootloader
INCLUDE_BOOTLOADER      ?= 0

# Specify beatstack config, single-hop if not set
LIBBEAT_CONFIG          ?= ""


LIBOTA_CONFIG          = 0

#app start
#if bootloader is included APP_START value is retrived from .board file
#with current bootloader APP_START should be 0x20000
ifeq ("$(INCLUDE_BOOTLOADER)", "0")
  APP_START = 0
  PROGRAM_IMAGE         ?= $(BUILD_DIR)/$(PROJECT_NAME).bin
else
  PROGRAM_IMAGE         ?= $(BUILD_DIR)/combo.bin
endif

# Common build options - some of these should be moved to targets/boards
CFLAGS                  += -Wall -std=c99
CFLAGS                  += -ffunction-sections -fdata-sections -ffreestanding -fsingle-precision-constant -Wstrict-aliasing=0
CFLAGS                  += -DconfigUSE_TICKLESS_IDLE=0
CFLAGS                  += -DUSE_CMSIS_OS2
CFLAGS                  += -D__START=main -D__STARTUP_CLEAR_BSS
CFLAGS                  += -DVTOR_START_LOCATION=$(APP_START) -Wl,--section-start=.text=$(APP_START)
LDFLAGS                 += -nostartfiles -Wl,--gc-sections -Wl,--relax -Wl,-Map=$(@:.elf=.map),--cref -Wl,--wrap=atexit
LDFLAGS                 += -Wl,--undefined=gHeaderData -Wl,--undefined=gHeaderSize
LDFLAGS                 += -Wl,--undefined=uxTopUsedPriority
LDLIBS                  += -lgcc
INCLUDES                += -Xassembler -I$(BUILD_DIR) -I.

# If set, disables asserts and debugging, enables optimization
RELEASE_BUILD           ?= 1

# Set the lll verbosity base level
CFLAGS                  += -DBASE_LOG_LEVEL=0xFFFF

# Enable debug messages
VERBOSE                 ?= 0
# Disable info messages
#SILENT                  ?= 1

# This project contains several Makefiles that reference the project root
ROOT_DIR                ?= $(abspath .)
ZOO                     ?= $(ROOT_DIR)/zoo
# Destination for build results
BUILD_BASE_DIR          ?= build
# Mark the default target
DEFAULT_BUILD_TARGET    ?= $(PROJECT_NAME)

# Configure how image is programmed to target device
PROGRAM_DEST_ADDR       ?= 0

# Flash header
FHEADER_DEST_ADDR        = $(FLASH_HEADER_LOC)

# Silabs SDK location and version, due to licensing terms, the SDK is not
# distributed with this project and must be installed with Simplicity Studio.
# The variable needs to point at the subdirectory with the version number, set
# it in Makefile.private or through the environment.
SILABS_SDKDIR           ?= $(HOME)/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.7

# Pull in the developer's private configuration overrides and settings
-include Makefile.private

# _______________________ Non-overridable configuration _______________________

BUILD_DIR                = $(BUILD_BASE_DIR)/$(BUILD_TARGET)
BUILDSYSTEM_DIR         := $(ZOO)/thinnect.node-buildsystem/make
PLATFORMS_DIRS          := $(ZOO)/thinnect.node-buildsystem/make $(ZOO)/thinnect.dev-platforms/make $(ZOO)/thinnect.hiot-platforms/make
PHONY_GOALS             := all clean
TARGETLESS_GOALS        += clean
UUID_APPLICATION        := b765981d-fb13-4644-be61-6c4f77f7c109

VERSION_BIN             := $(shell printf "%02X" $(VERSION_MAJOR))$(shell printf "%02X" $(VERSION_MINOR))$(shell printf "%02X" $(VERSION_PATCH))
VERSION_STR             := "$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)"$(VERSION_DEVEL)
SW_MAJOR_VERSION        := $(VERSION_MAJOR)
SW_MINOR_VERSION        := $(VERSION_MINOR)
SW_PATCH_VERSION        := $(VERSION_PATCH)
BUILD_TIMESTAMP         := $(shell date '+%s')
IDENT_TIMESTAMP         := $(BUILD_TIMESTAMP)

# NODE_PLATFORM_DIR is used by targets to add components to INCLUDES and SOURCES
NODE_PLATFORM_DIR       := $(ZOO)/thinnect.node-platform

# ______________ Build components - sources and includes _______________________

SOURCES += main.c

SOURCES += dummy_node_coordinates.c

MIST_LIGHT_CONTROL ?= 1
ifneq ($(MIST_LIGHT_CONTROL),0)
  CFLAGS += -DEXAMPLE_MIST_LIGHT_CONTROL
  SOURCES += mist_mod_lighting.c
endif

MIST_MOVEMENT ?= 1
ifneq ($(MIST_MOVEMENT),0)
  CFLAGS += -DEXAMPLE_MIST_MOVEMENT
  SOURCES += mist_mod_movement.c
endif

MIST_BUTTON ?= 0
ifneq ($(MIST_BUTTON),0)
  CFLAGS += -DEXAMPLE_MIST_BUTTON
  SOURCES += mist_mod_button.c
endif

MIST_LUX ?= 0
ifneq ($(MIST_LUX),0)
  CFLAGS += -DEXAMPLE_MIST_LUX
  SOURCES += mist_mod_lux.c
endif

# FreeRTOS
FREERTOS_DIR ?= $(ZOO)/FreeRTOS-Kernel
FREERTOS_INC = -I$(FREERTOS_DIR)/include \
               -I$(ZOO)/thinnect.cmsis-freertos/CMSIS_5/CMSIS/RTOS2/Include \
               -I$(ZOO)/thinnect.cmsis-freertos/CMSIS-FreeRTOS/CMSIS/RTOS2/FreeRTOS/Include \
               -I$(ZOO)/thinnect.cmsis-freertos/$(MCU_ARCH)

FREERTOS_SRC = $(wildcard $(FREERTOS_DIR)/*.c) \
               $(ZOO)/thinnect.cmsis-freertos/CMSIS-FreeRTOS/CMSIS/RTOS2/FreeRTOS/Source/cmsis_os2.c

INCLUDES += $(FREERTOS_PORT_INC) $(FREERTOS_INC)
SOURCES += $(FREERTOS_PORT_SRC) $(FREERTOS_SRC)

# CMSIS_CONFIG_DIR is used to add default CMSIS and FreeRTOS configs to INCLUDES
CMSIS_CONFIG_DIR ?= $(ZOO)/thinnect.cmsis-freertos/$(MCU_ARCH)/config

INCLUDES += -I$(ZOO)/thinnect.cmsis-ext
SOURCES += $(ZOO)/thinnect.cmsis-ext/cmsis_os2_ext.c
SOURCES += $(ZOO)/thinnect.cmsis-ext/freertos/cmsis_os2_ext_info.c

# Silabs EMLIB, RAIL, radio
INCLUDES += \
    -I$(SILABS_SDKDIR)/hardware/kit/common/drivers \
    -I$(SILABS_SDKDIR)/platform/halconfig/inc/hal-config \
    -I$(SILABS_SDKDIR)/platform/emlib/inc \
    -I$(SILABS_SDKDIR)/platform/emdrv/common/inc \
    -I$(SILABS_SDKDIR)/platform/emdrv/sleep/inc \
    -I$(SILABS_SDKDIR)/platform/emdrv/dmadrv/inc \
    -I$(SILABS_SDKDIR)/platform/emdrv/dmadrv/config \
    -I$(SILABS_SDKDIR)/platform/radio/rail_lib/hal \
    -I$(SILABS_SDKDIR)/platform/radio/rail_lib/common \
    -I$(SILABS_SDKDIR)/platform/radio/rail_lib/protocol/ieee802154 \
    -I$(SILABS_SDKDIR)/platform/radio/rail_lib/plugin/pa-conversions

SOURCES += \
    $(SILABS_SDKDIR)/hardware/kit/common/drivers/retargetserial.c \
    $(SILABS_SDKDIR)/hardware/kit/common/drivers/retargetio.c \
    $(SILABS_SDKDIR)/platform/emlib/src/em_system.c \
    $(SILABS_SDKDIR)/platform/emlib/src/em_core.c \
    $(SILABS_SDKDIR)/platform/emlib/src/em_emu.c \
    $(SILABS_SDKDIR)/platform/emlib/src/em_cmu.c \
    $(SILABS_SDKDIR)/platform/emlib/src/em_rmu.c \
    $(SILABS_SDKDIR)/platform/emlib/src/em_gpio.c \
    $(SILABS_SDKDIR)/platform/emlib/src/em_i2c.c \
    $(SILABS_SDKDIR)/platform/emlib/src/em_adc.c \
    $(SILABS_SDKDIR)/platform/emlib/src/em_iadc.c \
    $(SILABS_SDKDIR)/platform/emlib/src/em_ldma.c \
    $(SILABS_SDKDIR)/platform/emlib/src/em_usart.c \
    $(SILABS_SDKDIR)/platform/emlib/src/em_msc.c \
    $(SILABS_SDKDIR)/platform/emlib/src/em_rtcc.c \
    $(SILABS_SDKDIR)/platform/emlib/src/em_timer.c \
    $(SILABS_SDKDIR)/platform/emlib/src/em_wdog.c \
    $(SILABS_SDKDIR)/platform/emlib/src/em_se.c \
    $(SILABS_SDKDIR)/platform/emdrv/sleep/src/sleep.c \
    $(SILABS_SDKDIR)/platform/emdrv/dmadrv/src/dmadrv.c \
    $(SILABS_SDKDIR)/platform/radio/rail_lib/hal/hal_common.c

# logging
#CFLAGS += -DLOGGER_TIMESTAMP
#CFLAGS  += -DLOGGER_FWRITE
#SOURCES += $(NODE_PLATFORM_DIR)/silabs/logger_fwrite.c
#CFLAGS  += -DLOGGER_LDMA_BUFFER_LENGTH=16384
CFLAGS  += -DLOGGER_LDMA -DLOGGER_LDMA_DMADRV
SOURCES += $(NODE_PLATFORM_DIR)/silabs/logger_ldma.c
SOURCES += $(NODE_PLATFORM_DIR)/silabs/logger_fwrite_basic.c
SOURCES += $(ZOO)/thinnect.lll/logging/loggers_ext.c
INCLUDES += -I$(ZOO)/thinnect.lll/logging

# Some common setup-scenario implementations
INCLUDES += -I$(NODE_PLATFORM_DIR)/widgets
SOURCES += $(NODE_PLATFORM_DIR)/widgets/basic_rtos_filesystem_setup.c
SOURCES += $(NODE_PLATFORM_DIR)/widgets/basic_rtos_logger_setup.c
SOURCES += $(NODE_PLATFORM_DIR)/widgets/basic_rtos_threads_stats.c

# device signature
INCLUDES += -I$(ZOO)/thinnect.device-signature/signature \
            -I$(ZOO)/thinnect.device-signature/area
SOURCES  += $(ZOO)/thinnect.device-signature/signature/DeviceSignature.c \
            $(ZOO)/thinnect.device-signature/area/silabs/SignatureArea.c

# device announcement
INCLUDES += -I$(ZOO)/thinnect.tos-deviceannouncement/include

SOURCES += $(ZOO)/thinnect.tos-deviceannouncement/src/announcement_app.c \
           $(ZOO)/thinnect.tos-deviceannouncement/src/device_announcement.c \
           $(ZOO)/thinnect.tos-deviceannouncement/src/device_features.c

# Generally useful external tools
INCLUDES += -I$(ZOO)/lammertb.libcrc/include \
            -I$(ZOO)/jtbr.endianness \
            -I$(ZOO)/graphitemaster.incbin
SOURCES += $(ZOO)/lammertb.libcrc/src/crcccitt.c

# spiffs
INCLUDES += -I$(ZOO)/pellepl.spiffs/src
SOURCES += $(ZOO)/pellepl.spiffs/src/spiffs_cache.c \
           $(ZOO)/pellepl.spiffs/src/spiffs_check.c \
           $(ZOO)/pellepl.spiffs/src/spiffs_gc.c \
           $(ZOO)/pellepl.spiffs/src/spiffs_hydrogen.c \
           $(ZOO)/pellepl.spiffs/src/spiffs_nucleus.c

# filesystem
INCLUDES += -I$(ZOO)/thinnect.node-filesystem
INCLUDES += -I$(ZOO)/thinnect.node-filesystem/config
SOURCES += $(ZOO)/thinnect.node-filesystem/fs.c

# mistcomm
INCLUDES += -I$(ZOO)/thinnect.mist-comm/include
INCLUDES += -I$(ZOO)/thinnect.mist-comm/include/compat
INCLUDES += -I$(ZOO)/thinnect.mist-comm/addrcache
INCLUDES += -I$(ZOO)/thinnect.mist-comm/am
INCLUDES += -I$(ZOO)/thinnect.mist-comm/cmsis
SOURCES += $(wildcard $(ZOO)/thinnect.mist-comm/am/*.c)
SOURCES += $(wildcard $(ZOO)/thinnect.mist-comm/api/*.c)
SOURCES += $(wildcard $(ZOO)/thinnect.mist-comm/addrcache/*.c)
SOURCES += $(wildcard $(ZOO)/thinnect.mist-comm/routing/*.c)
SOURCES += $(wildcard $(ZOO)/thinnect.mist-comm/cmsis/*.c)
SOURCES += $(wildcard $(ZOO)/thinnect.mist-comm/control/*.c)

# platform stuff - watchdog, io etc...
INCLUDES += -I$(NODE_PLATFORM_DIR)/include
SOURCES += $(NODE_PLATFORM_DIR)/common/platform_mutex.c
SOURCES += $(NODE_PLATFORM_DIR)/common/spi_flash.c
SOURCES += $(NODE_PLATFORM_DIR)/common/radio_seqNum.c
SOURCES += $(NODE_PLATFORM_DIR)/common/eui64.c
SOURCES += $(NODE_PLATFORM_DIR)/common/sys_panic.c
SOURCES += $(NODE_PLATFORM_DIR)/common/time_rtc.c
SOURCES += $(NODE_PLATFORM_DIR)/common/yxktime.c
SOURCES += $(NODE_PLATFORM_DIR)/common/ident_parameters.c

INCLUDES += -I$(NODE_PLATFORM_DIR)/include/silabs
SOURCES += $(NODE_PLATFORM_DIR)/silabs/radio_rtos.c
SOURCES += $(NODE_PLATFORM_DIR)/silabs/retargetspi.c
SOURCES += $(NODE_PLATFORM_DIR)/silabs/retargeti2c.c
SOURCES += $(NODE_PLATFORM_DIR)/silabs/watchdog.c

# mist library
INCLUDES += -I$(ROOT_DIR)/libmist/
LDLIBS   += $(ROOT_DIR)/libmist/$(MCU_FAMILY)/libmistmiddleware.a

#libota
ifneq ($(LIBOTA_CONFIG),"")
    ifneq ("$(wildcard libota/updater.h)","")
        $(info libota found and included)
        ifeq ("$(INCLUDE_BOOTLOADER)", "1")
	          INCLUDES += -I$(ROOT_DIR)/libota/
	          LDLIBS += $(ROOT_DIR)/libota/$(MCU_FAMILY)/libota.a
        else
            $(error "ERROR: ota enabled and included but bootloader missing")
        endif
    else
	      ifneq ($(MAKECMDGOALS),clean)
            $(error "ERROR: libota enabled but not found")
        endif
    endif
endif

#beatsack
ifneq ($(LIBBEAT_CONFIG),"")
    ifneq ("$(wildcard libbeat/$(LIBBEAT_CONFIG)/beatstack.h)","")
        $(info "libbeat found and included")
        INCLUDES += -I$(ROOT_DIR)/libbeat/$(LIBBEAT_CONFIG)/
        LDLIBS += $(ROOT_DIR)/libbeat/$(LIBBEAT_CONFIG)/$(MCU_FAMILY)/libbeat.a
        SOURCES += $(NODE_PLATFORM_DIR)/widgets/basic_rtos_beatstack_timesync.c
        CFLAGS += -DINCLUDE_BEATSTACK
    else
        ifneq ($(MAKECMDGOALS),clean)
            $(error "ERROR: libbeat enabled but not found")
        endif
    endif
endif

# MoteXML components
MOTEXML_DIR ?= $(ZOO)/prolab.motexml
DTTYPES_DIR ?= $(ZOO)/prolab.dt-types
INCLUDES += -I$(MOTEXML_DIR)/lib -I$(DTTYPES_DIR)
SOURCES += $(MOTEXML_DIR)/lib/MLE.c $(MOTEXML_DIR)/lib/MLD.c $(MOTEXML_DIR)/lib/MLI.c
CFLAGS += -DLIBEXPORT=""
# ------------------------------------------------------------------------------

# Pull in the grunt work
include $(BUILDSYSTEM_DIR)/Makerules

# for whatever reason libm must come after the libraries that use it, so make sure it is last
LDLIBS += -lm
# ------------------------------------------------------------------------------

# Print some build parameters
$(info NODE_AM_ADDR=$(NODE_AM_ADDR))
$(info DEFAULT_RADIO_CHANNEL=$(DEFAULT_RADIO_CHANNEL))
$(info DEFAULT_PAN_ID=$(DEFAULT_PAN_ID))
$(info DEFAULT_RFPOWER_DBM=$(DEFAULT_RFPOWER_DBM))
# ------------------------------------------------------------------------------

$(call passVarToCpp,CFLAGS,VERSION_MAJOR)
$(call passVarToCpp,CFLAGS,VERSION_MINOR)
$(call passVarToCpp,CFLAGS,VERSION_PATCH)
$(call passVarToCpp,CFLAGS,VERSION_STR)
$(call passVarToCpp,CFLAGS,SW_MAJOR_VERSION)
$(call passVarToCpp,CFLAGS,SW_MINOR_VERSION)
$(call passVarToCpp,CFLAGS,SW_PATCH_VERSION)
$(call passVarToCpp,CFLAGS,IDENT_TIMESTAMP)

$(call passVarToCpp,CFLAGS,NODE_AM_ADDR)
$(call passVarToCpp,CFLAGS,DEFAULT_RADIO_CHANNEL)
$(call passVarToCpp,CFLAGS,DEFAULT_PAN_ID)

UUID_APPLICATION_BYTES = $(call uuidToCstr,$(UUID_APPLICATION))
$(call passVarToCpp,CFLAGS,UUID_APPLICATION_BYTES)

$(call passVarToCpp,CFLAGS,BASE_LOG_LEVEL)

# _______________________________ Project rules _______________________________

all: $(BUILD_DIR)/$(PROJECT_NAME).bin

# header.bin should be recreated if a build takes place
$(OBJECTS): $(BUILD_DIR)/header.bin

$(BUILD_DIR)/$(PROJECT_NAME).elf: Makefile | $(BUILD_DIR)

$(BUILD_DIR)/header.bin: Makefile | $(BUILD_DIR)
	$(call pInfo,Creating application header block [$@])
	$(HEADEREDIT) -c -v softtype,1 -v firmaddr,$(APP_START) -v firmsizemax,$(APP_MAX_LEN) \
	    -v version,$(VERSION_STR) -v versionbin,$(VERSION_BIN) \
	    -v uuid,$(UUID_BOARD) -v uuid2,$(UUID_PLATFORM) -v uuid3,$(UUID_APPLICATION) \
	    -v timestamp,$(BUILD_TIMESTAMP) \
	    -v name,$(PROJECT_NAME) \
	    -v size -v crc "$@"

$(BUILD_DIR)/$(PROJECT_NAME).elf: $(OBJECTS)
	$(call pInfo,Linking [$@])
	$(HIDE_CMD)$(CC) $(CFLAGS) $(INCLUDES) $(OBJECTS) $(LDFLAGS) $(LDLIBS) -o $@

$(BUILD_DIR)/$(PROJECT_NAME).bin: $(BUILD_DIR)/$(PROJECT_NAME).elf
	$(call pInfo,Exporting [$@])
	$(HIDE_CMD)$(TC_SIZE) --format=Berkeley $<
	$(HIDE_CMD)$(TC_OBJCOPY) --strip-all -O binary "$<" "$@"
	$(HIDE_CMD)$(HEADEREDIT) -v size -v crc $@

$(BUILD_DIR)/combo.bin: bootloader/wfs201-bootloader.bin $(BUILD_DIR)/$(PROJECT_NAME).bin
	$(call pInfo,Building combo [$@])
	srec_cat bootloader/wfs201-bootloader.bin -binary -offset $(BOOTLOADER_START) \
	                  $(BUILD_DIR)/$(PROJECT_NAME).bin -binary -offset $(APP_START) \
	                  -o $@ -binary
	chmod 755 "$@"

ifeq ("$(INCLUDE_BOOTLOADER)", "1")
$(PROJECT_NAME): $(BUILD_DIR)/combo.bin
else
$(PROJECT_NAME): $(BUILD_DIR)/$(PROJECT_NAME).bin
endif

# _______________________________ Utility rules ________________________________

$(BUILD_DIR):
	$(call pInfo,Creating [$@])
	@mkdir -p "$@"

clean:
	$(call pInfo,Nuking everything in [$(BUILD_BASE_DIR)])
	@-rm -rf "$(BUILD_BASE_DIR)"

.PHONY: $(PHONY_GOALS)
