export SRC_TOP_DIR ?= $(shell pwd)/../..
export MAKE_TOP_DIR ?= $(SRC_TOP_DIR)/mk

INC ?= -I$(SRC_TOP_DIR)/include

BUILD_DIR ?= ./build

DFLAGS += -DRRLLIB_LOG
CFLAGS +=  -g -Wall -O3 -finline-functions -m64 

INSTALL_INCS += rrllib.h
INSTALL_INCS += rrllib.x

SRCS		= rrllib_main.c \
			  rrllib_path.c \
			  rrllib_doc.c \
			  rrllib_parse.c \
			  rrllib_query.c \
			  rrllib_mthod.c \
			  rrllib_glob.c \

INSTALL_DIR ?= $(SRC_TOP_DIR)

OUTPUT_LIB ?= librrl.a

include $(MAKE_TOP_DIR)/lib.mk

