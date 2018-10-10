export SRC_TOP_DIR ?= $(shell pwd)/../..
export MAKE_TOP_DIR ?= $(SRC_TOP_DIR)/mk

INC ?= -I$(SRC_TOP_DIR)/include

INSTALL_INCS +=gendef.h
INSTALL_INCS +=smd_user.h
INSTALL_INCS +=smd_user.x

INSTALL_DIR ?= $(SRC_TOP_DIR)

include $(MAKE_TOP_DIR)/lib.mk
