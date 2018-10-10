export SRC_TOP_DIR ?= $(shell pwd)/../..
export MAKE_TOP_DIR ?= $(SRC_TOP_DIR)/mk

INC ?= -I$(SRC_TOP_DIR)/include

CFLAGS +=  -g -Wall -O3 -finline-functions -m64

INSTALL_INCS += rcllib.h
INSTALL_INCS += rcllib.x

SRCS		= rcllibInt_glob.c \
			  rcllib_api.c

INSTALL_DIR ?= $(SRC_TOP_DIR)

OUTPUT_LIB ?= librcl.a

include $(MAKE_TOP_DIR)/lib.mk
