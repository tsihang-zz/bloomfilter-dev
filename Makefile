VER = $(shell cat package/rulemonitor-info.yaml | grep " version:" | cut -b 12-15)
BUILD   = $(shell git log | grep -cE 'Author:')
BSTR = $(shell printf %03d $(BUILD))

ARCH:=unix
TARGET = bloomfilter

COMPILE = gcc
CPPCOMPILE = g++

LIB_PATH = ../../libs
LIBS += -ldl -lpthread -lm -lstdc++\
	-L $(LIB_PATH) -laprutil-1\
	-L $(LIB_PATH) -lapr-1
	

OBJS_LOCAL = oryx_bloomfilter.o\
			oryx_list.o\
			oryx_ipc.o\
			oryx_assert.o\
			oryx_rbtree.o

CFLAGS_LOCAL := -std=gnu99 -W -Wall -Wunused-parameter -g -O3\
			-I ./third_party/apr/apr\
			-I ./third_party/apr/apr/arch/$(ARCH)\
			-I ./third_party/apr/apr-util

.PHONY: compile clean


$(TARGET): $(OBJS_LOCAL) 
	$(COMPILE) -o $@ $(OBJS_LOCAL) $(LIBS)

$(OBJS_LOCAL): %.o : %.c
	$(COMPILE) $(CFLAGS_LOCAL) -c $< -o $@

$(CPP_OBJS_LOCAL): %.o : %.cpp
	$(CPPCOMPILE) $(CFLAGS_LOCAL) -c $< -o $@

clean:
	rm -rf $(OBJS_LOCAL) $(CPP_OBJS_LOCAL) $(TARGET) 


