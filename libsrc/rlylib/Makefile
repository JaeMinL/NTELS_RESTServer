export SRC_TOP_DIR ?= $(shell pwd)/../..
export MAKE_TOP_DIR ?= $(SRC_TOP_DIR)/mk

INC ?= -I$(SRC_TOP_DIR)/include

BUILD_DIR ?= ./build

DFLAGS += -DRLYLIB_LOG
CFLAGS += -g -Wall -O3 -finline-functions -m64 

INSTALL_INCS += rlylib.h
INSTALL_INCS += rlylib.x

SRCS		= rlylib_api.c \
			  rlylibInt_acpt.c \
			  rlylibInt_glob.c \
			  rlylibInt_host.c \
			  rlylibInt_init.c \
			  rlylibInt_dstry.c \
			  rlylibInt_msg.c \
			  rlylibInt_rlm.c \
			  rlylibInt_sync.c \

INSTALL_DIR ?= $(SRC_TOP_DIR)

OUTPUT_LIB ?= librly.a

include $(MAKE_TOP_DIR)/lib.mk

