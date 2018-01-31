#
# See the file LICENSE for redistribution information.
#
# Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
# All rights reserved.
#

# Global Variable definitions.
PROJECT_ROOT := $(shell pwd)/
SOURCE_ROOT  := $(PROJECT_ROOT)src/
BUILD_ROOT   := $(PROJECT_ROOT)build/
TARGET_ROOT  := $(BUILD_ROOT)bin/
OBJECT_ROOT  := $(BUILD_ROOT)obj/

UNITTEST_ROOT   := $(PROJECT_ROOT)unit-test/
MODULETEST_ROOT := $(PROJECT_ROOT)module-test/

$(shell mkdir -p ${BUILD_ROOT})
$(shell mkdir -p ${TARGET_ROOT})
$(shell mkdir -p ${OBJECT_ROOT})

export PROJECT_ROOT
export SOURCE_ROOT
export TARGET_ROOT
export OBJECT_ROOT
export UNITTEST_ROOT

COMMON_MK  := $(PROJECT_ROOT)common.mk
DEFINES_MK := $(PROJECT_ROOT)makedefs.mk
OBJECT_PATH_MK := $(PROJECT_ROOT)objectpath.mk
export COMMON_MK DEFINES_MK OBJECT_PATH_MK

DATA_DIR := $(PROJECT_ROOT)data

core:
	$(MAKE) -C $(SOURCE_ROOT)

utest:
	$(MAKE) -C $(UNITTEST_ROOT)

mtest:
	$(MAKE) -C $(MODULETEST_ROOT)

all: core utest mtest

cert:
	-cp -rf $(DATA_DIR)/.certs $(TARGET_ROOT)

clean:
	$(MAKE) -C $(UNITTEST_ROOT) clean
	$(MAKE) -C $(MODULETEST_ROOT) clean
	-rm -rf $(BUILD_ROOT)
	@echo "====== ${BUILD_OUT} has been removed!!! Cleanup Done. "

.PHONY: all core utest install clean
