#
# See the file LICENSE for redistribution information.
#
# Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
# All rights reserved.
#

OBJ_DIR := $(TEST_BUILD_DIR)
CFLAGS  += -I../include

# Need uncommented if debug the memory leak issue.
#CFLAGS += -include ../include/CppUTest/MemoryLeakDetectorNewMacros.h
#CXXFLAGS += -include ../include/CppUTest/MemoryLeakDetectorNewMacros.h

export CFLAGS OBJ_DIR

include $(DEFINES_MK)
include $(COMMON_MK)

TEST_APP ?= testAll

all:
	$(CXX) -o $(OBJ_DIR)/$(TEST_APP) \
		$(OBJ_DIR)/*.o \
		$(OBJECT_ROOT)Memory/*.o \
		$(OBJECT_ROOT)Common/*.o \
		$(OBJECT_ROOT)HTTPBase/*.o \
		$(OBJECT_ROOT)HTTP/*.o \
		$(OBJECT_ROOT)Thread/*.o \
		$(OBJECT_ROOT)IO/*.o \
		$(OBJECT_ROOT)TLS/*.o \
		$(OBJECT_ROOT)DataCom/*.o \
		$(OBJECT_ROOT)DataBase/*.o \
		$(OBJECT_ROOT)Network/*.o \
		$(OBJECT_ROOT)URI/*.o \
		$(OBJECT_ROOT)XML/*.o \
		$(OBJECT_ROOT)Compress/*.o \
		$(OBJECT_ROOT)Config/*.o \
		$(OBJECT_ROOT)Stream/*.o \
		$(OBJECT_ROOT)Tracker/*.o \
		../lib/libCppUTest.a \
		../lib/libCppUTestExt.a \
		-Xlinker "-(" $(LDFLAGS) -Xlinker "-)" -rdynamic
	@echo "====> Generated Binary File: <${OBJ_DIR}/${TEST_APP}> Successed!!!"

.PHONY: all