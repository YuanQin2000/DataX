#
# See the file LICENSE for redistribution information.
#
# Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
# All rights reserved.
#

# Common Make Rules

SUB_DIRS  := $(patsubst ./%/Makefile, %, \
                 $(shell find -maxdepth 1 -type d -a -regex './.*' \
                     -exec find {} -maxdepth 1 -name 'Makefile' \;))

SOURCE_C   := $(wildcard *.c)
SOURCE_CPP := $(wildcard *.cpp)
CUR_OBJS   := $(patsubst %.c, %.o, $(SOURCE_C))
CUR_OBJS   += $(patsubst %.cpp, %.o, $(SOURCE_CPP))
CUR_OBJS   := $(patsubst %,$(OBJ_DIR)/%, $(CUR_OBJS))


all: $(SUB_DIRS) $(CUR_OBJS)

$(SUB_DIRS):
	@$(MAKE) -C $@


ifdef OBJ_DIR

$(shell mkdir -p ${OBJ_DIR})

DEPS_C   := $(patsubst %.c, $(OBJ_DIR)/%.d, $(SOURCE_C))
DEPS_CPP := $(patsubst %.cpp, $(OBJ_DIR)/%.d, $(SOURCE_CPP))

$(OBJ_DIR)/%.d:%.c
	$(CC) $(CFLAGS) -MM $< -o $@
	@sed -i 's#\(.*\)\.o[ :]*#\1.o $@ : #g' $@

$(OBJ_DIR)/%.d:%.cpp
	$(CXX) $(CXXFLAGS) -MM $< -o $@
	@sed -i 's#\(.*\)\.o[ :]*#\1.o $@ : #g' $@

$(OBJ_DIR)/%.o:%.c $(OBJ_DIR)/%.d
	@echo "CC    $<"
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o:%.cpp $(OBJ_DIR)/%.d
	@echo "CC    $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

sinclude $(DEPS_C)
sinclude $(DEPS_CPP)

endif

debug:
	@echo "Under DIR: ${shell pwd}"
	@echo "MAKELEVEL: ${MAKELEVEL}"
	@echo "PROJECT_ROOT: ${PROJECT_ROOT}"
	@echo "SOURCE_ROOT: ${SOURCE_ROOT}"
	@echo "TARGET_ROOT: ${TARGET_ROOT}"
	@echo "OBJECT_ROOT: ${OBJECT_ROOT}"

.PHONY: all cleanup $(SUB_DIRS)
