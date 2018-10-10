export SRC_TOP_DIR ?= $(shell pwd)/../..
export MAKE_TOP_DIR ?= $(SRC_TOP_DIR)/mk

INC ?= -I$(SRC_TOP_DIR)/include
INC += -I/usr/include/libxml2 
INC += -I/usr/local/opt/libxml2/include/libxml2/ 

BUILD_DIR ?= ./build

DFLAGS += -DLOGLIB_ENB_LVL=3
DFLAGS += -DLOGLIB_DISP_ENV_LVL 
DFLAGS += -DLOGLIB_DISP_ENV_FILE 
DFLAGS += -DLOGLIB_DISP_ENV_LINE
DFLAGS += -DLOGLIB_DISP_ENV_TIME
#DFLAGS += -DLOGLIB_TOML_CONFIG_DISABLE
#DFLAGS += -DLOGLIB_XML_CONFIG_DISABLE
DFLAGS += -DLOGLIB_LOG

CFLAGS +=  -g -Wall -O3 -finline-functions -m64 

INSTALL_INCS += loglib.h
INSTALL_INCS += loglib.x

SRCS		= loglibInt_init.c \
			  loglibInt_apnd.c\
			  loglibInt_thrd.c\
			  loglibInt_misc.c\
			  loglibInt_load.c\
			  loglibInt_loadXml.c\
			  loglibInt_loadToml.c\
			  loglibInt_glob.c\
			  loglib_api.c\

INSTALL_DIR ?= $(SRC_TOP_DIR)

OUTPUT_LIB ?= liblog.a

include $(MAKE_TOP_DIR)/lib.mk
