export SRC_TOP_DIR ?= $(shell pwd)/../..
export MAKE_TOP_DIR ?= $(SRC_TOP_DIR)/mk

INC ?= -I$(SRC_TOP_DIR)/include

BUILD_DIR ?= ./build

DFLAGS += #-DTRNLIB_LOG
CFLAGS +=  -g -Wall -O3 -finline-functions -m64 

INSTALL_INCS += trnlib.h
INSTALL_INCS += trnlib.x

SRCS		= trnlib_sock.c \
			  trnlib_util.c \

INSTALL_DIR ?= $(SRC_TOP_DIR)

OUTPUT_LIB ?= libtrn.a

include $(MAKE_TOP_DIR)/lib.mk

