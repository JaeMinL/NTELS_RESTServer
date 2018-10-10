export SRC_TOP_DIR ?= $(shell pwd)/../..
export MAKE_TOP_DIR ?= $(SRC_TOP_DIR)/mk

INC ?= -I$(SRC_TOP_DIR)/include

BUILD_DIR ?= ./build

CFLAGS +=  -g -Wall -O3 -finline-functions -m64 

INSTALL_INCS += rsvlib.h
INSTALL_INCS += rsvlib.x

SRCS		= rsvlibInt_glob.c \
			  rsvlibInt_init.c \
			  rsvlibInt_main.c \
			  rsvlibInt_svr.c \
			  rsvlibInt_util.c \
			  rsvlib_api.c

INSTALL_DIR ?= $(SRC_TOP_DIR)

OUTPUT_LIB ?= librsv.a

include $(MAKE_TOP_DIR)/lib.mk

