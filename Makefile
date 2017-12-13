
TARGET = app

MAJOR="1.0"
BUILD=$(shell git log | grep -cE 'Author:.*<.*@.*>')
BUILDSHA=$(shell git rev-parse --short HEAD)
BUILD_FINGERPRINT=$(MAJOR).$(BUILD).$(BUILDSHA)

# Default simulation parameters
NUM_PROCESSORS=1

TARGET_LIB = $(OBJ_DIR)/liboryx.a

all: ${TARGET}

#  standard common Makefile fragment
include $(APPL_SDK_HOME)/config.mk
include $(APPL_SDK_HOME)/common.mk

# 3RD library path
LIB_PATH := $(APPL_SDK_HOME)/deps
# APR (Apache Portable Runtime)

#  application specification

dir := .
OBJS := \
	$(OBJ_DIR)/bf.o\
	$(OBJ_DIR)/oryx_bf.o\
	$(OBJ_DIR)/oryx_cbf.o

dir := $(APPL_SDK_HOME)/lib
include $(dir)/lib.mk
OBJS += $(OBJS_$(dir))

LDFLAGS_GLOBAL += \
		-lpthread -ldl -lcrypt -lm\
		-L$(LIB_PATH) -lmysqlclient\
		-L$(LIB_PATH) -lapr-1\
		-L$(LIB_PATH) -laprutil-1\
		-L$(LIB_PATH) -lsndfile\
		-L$(LIB_PATH) -lpcre\
		-L$(LIB_PATH) -ljson\
		-L$(LIB_PATH) -lcurl\
		-L$(LIB_PATH) -lpcap\
		-L$(LIB_PATH) -lyaml\
		#-L$(LIB_PATH) -lspandsp\

CFLAGS_LOCAL = --std=gnu99 -g -O3 -W -Wall -Wunused-parameter\
               -I.\
               -I./lib\
               -I./lib/include\
               -I./lib/apr/apr\
               -I./lib/apr/apr/arch/$(ARCH)\
               -I./lib/apr/apr-util\
               -I./components/json\
               -I./components/yaml\
               -I./components/inotify\
               -I./components/spandsp\
               -I./components/mysql\
               -I./components/zebra

DEPS   :=  $(objs:.o=.d)

CLEAN_LIST  :=  $(CLEAN_LIST) $(OBJS) $(DEPS) $(TARGET_LIB) $(OBJ_DIR)

tarball_lib: $(OBJS)
	ar rcs $(TARGET_LIB) $^
	sh script/tarball_lib.sh $(TARGET_LIB) $(MAJOR).$(BUILD) $(BUILDSHA)

tarball:
	sh script/tarball_all.sh $(TARGET) $(MAJOR).$(BUILD) $(BUILDSHA)

$(OBJ_DIR)/%.o: $(d)/%.c
	$(COMPILE)

include $(APPL_SDK_HOME)/application.mk

clean: 
	rm -rf $(CLEAN_LIST)
	rm -f $(TARGET)
	rm -rf logs

clobber: clean
	rm -rf $(OBJ_DIR)

