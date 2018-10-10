export SRC_TOP_DIR ?= $(shell pwd)/../..
export MAKE_TOP_DIR ?= $(SRC_TOP_DIR)/mk

INC ?= -I$(SRC_TOP_DIR)/include

BUILD_DIR ?= ./build

#DFLAGS += -DHTPLIB_LOG
CFLAGS +=  -g -Wall -O3 -finline-functions -m64 

INSTALL_INCS += htplib.h
INSTALL_INCS += htplib.x

SRCS		= htplib_init.c \
			  htplib_main.c \
			  htplib_staCode.c \
			  htplib_mthod.c \

INSTALL_DIR ?= $(SRC_TOP_DIR)

OUTPUT_LIB ?= libhtp.a

include $(MAKE_TOP_DIR)/lib.mk

