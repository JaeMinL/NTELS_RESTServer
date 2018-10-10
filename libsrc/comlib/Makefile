export SRC_TOP_DIR ?= $(shell pwd)/../..
export MAKE_TOP_DIR ?= $(SRC_TOP_DIR)/mk

INC ?= -I$(SRC_TOP_DIR)/include

BUILD_DIR ?= ./build

DFLAGS += -DGEN_CHK_ERR #-DCOMLIB_LOG
CFLAGS +=  -g -Wall -O3 -finline-functions -m64 

INSTALL_INCS += comlib.h
INSTALL_INCS += comlib.x

SRCS		= comlib_hash.c \
			  comlib_hashTbl.c \
			  comlib_lnkLst.c \
			  comlib_mem.c \
			  comlib_msg.c \
			  comlib_str.c \
			  comlib_util.c \
			  comlib_file.c \
			  comlib_timer.c

INSTALL_DIR ?= $(SRC_TOP_DIR)

OUTPUT_LIB ?= libcom.a

include $(MAKE_TOP_DIR)/lib.mk
