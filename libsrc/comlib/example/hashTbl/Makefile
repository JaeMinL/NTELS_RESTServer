export SRC_TOP_DIR ?= $(shell pwd)/../../../..
export MAKE_TOP_DIR ?= $(SRC_TOP_DIR)/mk

INC ?= -I$(SRC_TOP_DIR)/include

BUILD_DIR ?= ./build

DFLAGS += -DGEN_CHK_ERR #-DCOMLIB_LOG
CFLAGS +=  -g -Wall -O3 -finline-functions -m64 

SRCS = exp_hashTbl.c \

LIB_DIRS += -L$(SRC_TOP_DIR)/lib

LIBS += -lcom

INSTALL_DIR ?= $(BUILD_DIR)/app

OUTPUT_APP ?= hashTbl

include $(MAKE_TOP_DIR)/app.mk
