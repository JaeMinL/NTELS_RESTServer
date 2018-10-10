CC			     ?= gcc
AR			     ?= ar
MKDIR            ?= mkdir
CP               ?= cp

OBJ_OUT_FILES = $(foreach OBJ_FILE,$(SRCS),$(BUILD_DIR)/$(subst .c,.o,$(OBJ_FILE)))

# Check build directory
MAKE_BUILD_DIR := $(wildcard $(BUILD_DIR))

ifeq ($(strip$(MAKE_BUILD_DIR)), )
MAKE_BUILD_DIR = $(BUILD_DIR)
endif

