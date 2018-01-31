#
# See the file LICENSE for redistribution information.
#
# Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
# All rights reserved.
#

include $(OBJECT_PATH_MK)
include $(COMMON_MK)

APP_NAME ?= ctrl-app

exe:
	$(CXX) -o $(TARGET_ROOT)$(APP_NAME) \
		$(OBJ_DIR)/*.o \
		$(OBJECT_ROOT)Memory/*.o \
		$(OBJECT_ROOT)Common/*.o \
		$(OBJECT_ROOT)IO/*.o \
		$(OBJECT_ROOT)Thread/*.o \
		$(OBJECT_ROOT)TLS/*.o \
		$(OBJECT_ROOT)CmdLine/*.o \
		$(OBJECT_ROOT)ClientIf/*.o \
		$(OBJECT_ROOT)Tracker/*.o \
		-Xlinker "-(" $(LDFLAGS) -Xlinker "-)" -rdynamic
ifeq ($(RELEASE),release)
	$(STRIP) $(TARGET_ROOT)$(CTRL_APP)
endif
	@echo "====> Generated Binary File: <${TARGET_ROOT}${CTRL_APP}> Successed!!!"

.PHONY: exe