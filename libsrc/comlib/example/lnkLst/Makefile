export SRC_TOP_DIR ?= $(shell pwd)/../../../..
export MAKE_TOP_DIR ?= $(SRC_TOP_DIR)/mk

INC ?= -I$(SRC_TOP_DIR)/include

BUILD_DIR ?= ./build

DFLAGS += -DGEN_CHK_ERR #-DCOMLIB_LOG
CFLAGS +=  -g -Wall -O3 -finline-functions -m64 

SRCS = exp_lnkLst.c \

LIB_DIRS += -L$(SRC_TOP_DIR)/lib

LIBS += -lcom

INSTALL_DIR ?= ./install

OUTPUT_APP ?= lnkLst

include $(MAKE_TOP_DIR)/app.mk
