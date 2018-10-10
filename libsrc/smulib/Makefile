export SRC_TOP_DIR ?= $(shell pwd)/../..
export MAKE_TOP_DIR ?= $(SRC_TOP_DIR)/mk

INC ?= -I$(SRC_TOP_DIR)/include

BUILD_DIR ?= ./build

DFLAGS += -DSMULIB_LOG
CFLAGS +=  -g -Wall -O3 -finline-functions -m64 

INSTALL_INCS += smulib.h
INSTALL_INCS += smulib.x

SRCS		= smulibInt_glob.c\
			  smulibInt_init.c\
			  smulibInt_lh.c\
			  smulibInt_main.c\
			  smulibInt_sig.c\
			  smulibInt_thBlk.c\
			  smulibInt_timer.c\
			  smulibInt_uh.c\
			  smulib_api.c

INSTALL_DIR ?= $(SRC_TOP_DIR)

OUTPUT_LIB ?= libsmu.a

include $(MAKE_TOP_DIR)/lib.mk
