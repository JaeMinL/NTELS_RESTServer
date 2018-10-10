export SRC_TOP_DIR ?= $(shell pwd)/../..
export MAKE_TOP_DIR ?= $(SRC_TOP_DIR)/mk

INC ?= -I$(SRC_TOP_DIR)/include

BUILD_DIR ?= ./build

DFLAGS += -DGEN_CHK_ERR #-DCOMLIB_LOG
CFLAGS +=  -g -Wall -O3 -finline-functions

INSTALL_INCS += thrlib.h
INSTALL_INCS += thrlib.x

SRCS		= thrlib_thrd.c \
			  thrlib_mutx.c \
			  thrlib_cond.c \
			  thrlib_tq.c \
			  thrlib_pool.c \

INSTALL_DIR ?= $(SRC_TOP_DIR)

OUTPUT_LIB ?= libthr.a

include $(MAKE_TOP_DIR)/lib.mk

