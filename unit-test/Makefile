#
# See the file LICENSE for redistribution information.
#
# Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
# All rights reserved.
#

TEST_BUILD_DIR := $(UNITTEST_ROOT)build

$(shell mkdir -p ${TEST_BUILD_DIR})

export TEST_BUILD_DIR

include $(COMMON_MK)

clean:
	-rm -rf $(TEST_BUILD_DIR)

.PHONY: clean