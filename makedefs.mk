#
# See the file LICENSE for redistribution information.
#
# Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
# All rights reserved.
#

# Global Compile Options
LD_LIBRARY_PATH := $(APP_CROSS_PATH)/usr/lib
#CROSS_ARCH      := armv7a-cros-linux-gnueabi-

ifeq ($(findstring $(APP_CROSS_PATH)/bin,$(PATH)),)
PATH  := $(PATH):$(APP_CROSS_PATH)/bin
endif

AR    := @$(CROSS_ARCH)ar
AS    := @$(CROSS_ARCH)as
CC    := @$(CROSS_ARCH)gcc
CXX   := @$(CROSS_ARCH)g++
STRIP := @$(CROSS_ARCH)strip
#CPU_TYPE := cortex-a9

export PATH
export LANG AR AS CC CXX STRIP
export LD_LIBRARY_PATH 


3RDPARTY_DIR      := $(PROJECT_ROOT)3rdparty
APP_OUT_DIR       := $(TARGET_ROOT)App

export 3RDPARTY_DIR APP_OUT_DIR


CFLAGS += -I$(3RDPARTY_DIR)/include
CFLAGS += -I$(SOURCE_ROOT)

#CFLAGS := -march=armv7-a
#CFLAGS += -mtune=$(CPU_TYPE)
#CFLAGS += -mlittle-endian
#CFLAGS += -mfpu=vfpv3
#CFLAGS += -mfloat-abi=hard
CFLAGS += -fstack-protector --param=ssp-buffer-size=4
CFLAGS += -fmessage-length=0
CFLAGS += -pipe
CFLAGS += -Wall
CFLAGS += -Wformat -Wformat-security

# The global symbol (function and data) located on individual sections
# in order to avoid unused symbols in target binary.
CFLAGS += -ffunction-sections
CFLAGS += -fdata-sections

# No common block in object file.
CFLAGS += -fno-common

# SW Infomation:
ifndef __SW_VERSION__
COMPILE_INFORMATION += $(shell whoami)
COMPILE_INFORMATION += $(shell date +"compile at 20%y-%m-%d %H:%M:%S on")
COMPILE_INFORMATION += $(shell uname -m -s -n)
__SW_VERSION__ := $(COMPILE_INFORMATION)
endif

CFLAGS += -D__SW_VERSION__="\"$(__SW_VERSION__)"\"

# LDFLAGS:
LDFLAGS  += -L$(APP_CROSS_PATH)/usr/lib
LDFLAGS  += -L$(3RDPARTY_DIR)/lib
LDFLAGS  += -lpthread
LDFLAGS  += -lrt
LDFLAGS  += -ldl
LDFLAGS  += -lexpat
LDFLAGS  += -lz
LDFLAGS  += -lcrypto

#SSL libraries selection
ifeq ($(SSL), ) # Not defined the SSL library, use openssl as the default.
CFLAGS   += -D__USE_OPEN_SSL__
LDFLAGS  += -lssl
else
ifeq ($(SSL), openssl)
CFLAGS   += -D__USE_OPEN_SSL__
LDFLAGS  += -lssl
else ifeq ($(SSL), mbedtls)
CFLAGS   += -D__USE_MBED_TLS__
LDFLAGS  += -lmbedcrypto
LDFLAGS  += -lmbedtls
LDFLAGS  += -lmbedx509
endif
endif

LDFLAGS  += -lreadline
LDFLAGS  += -ldb

ifeq ($(RELEASE),release)
CFLAGS += -O2
else
CLFAGS += -O0
CFLAGS += -D__DEBUG__
CFLAGS += -g

#gcov, used for unit test coverage tool
CFLAGS += --coverage -fno-inline -fno-inline-small-functions -fno-default-inline -fprofile-arcs -ftest-coverage
LDFLAGS += -lgcov
endif

# CXXFLAGS:
CXXFLAGS := $(CFLAGS)
CXXFLAGS += -fno-rtti
CXXFLAGS += -fno-exceptions
CXXFLAGS += -std=c++11

export CFLAGS CXXFLAGS LDFLAGS
