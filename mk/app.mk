include $(MAKE_TOP_DIR)/global.pre.mk

INSTALL_APP_DIR  ?= $(INSTALL_DIR)/app

# build
all: $(MAKE_BUILD_DIR) $(BUILD_DIR)/$(OUTPUT_APP)

$(MAKE_BUILD_DIR):
	$(MKDIR) -p $(BUILD_DIR)

$(BUILD_DIR)/%.o:  %.c
	$(CC) $(CFLAGS) $(DFLAGS) $(INC) -c $< -o $@

$(BUILD_DIR)/$(OUTPUT_APP): $(OBJ_OUT_FILES)
	$(CC) $(CFLAGS) $(INC_DIRS) $(LIB_DIRS) $(OBJ_OUT_FILES) $(LIBS) -o $@ 

# install
# Check application directory
MAKE_INSTALL_APP_DIR := $(wildcard $(INSTALL_APP_DIR))

ifeq ($(strip$(MAKE_INSTALL_APP_DIR)), )
MAKE_INSTALL_APP_DIR = $(INSTALL_APP_DIR)
endif

install: $(MAKE_INSTALL_APP_DIR)
	$(CP) $(BUILD_DIR)/$(OUTPUT_APP) $(INSTALL_APP_DIR)

$(MAKE_INSTALL_APP_DIR):
	$(MKDIR) -p $(INSTALL_APP_DIR)

clean: 
	rm -rf $(OBJ_OUT_FILES) $(OUTPUT_APP) core

dep: 
	makedepend -- $(CFLAGS) -- $(SRCS)

