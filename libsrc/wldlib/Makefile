export SRC_TOP_DIR ?= $(shell pwd)/../..
export MAKE_TOP_DIR ?= $(SRC_TOP_DIR)/mk

INC ?= -I$(SRC_TOP_DIR)/include

BUILD_DIR ?= ./build

CFLAGS +=  -g -Wall -O3 -finline-functions -m64 

INSTALL_INCS += wldlib.h
INSTALL_INCS += wldlib.x

SRCS		= wldlib_init.c \
			  wldlib_main.c \
			  wldlib_blkLst.c \
			  wldlib_chr.c \
			  wldlib_parse.c \

INSTALL_DIR ?= $(SRC_TOP_DIR)

OUTPUT_LIB ?= libwld.a

include $(MAKE_TOP_DIR)/lib.mk
