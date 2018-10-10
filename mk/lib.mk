include $(MAKE_TOP_DIR)/global.pre.mk

INSTALL_LIB_DIR  ?= $(INSTALL_DIR)/lib
INSTALL_INC_DIR  ?= $(INSTALL_DIR)/include

# build
all: $(MAKE_BUILD_DIR) $(OBJ_OUT_FILES) $(BUILD_DIR)/$(OUTPUT_LIB)

$(MAKE_BUILD_DIR):
	$(MKDIR) -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) $(DFLAGS) $(INC) -c $< -o $@

$(BUILD_DIR)/$(OUTPUT_LIB): 
	$(AR) rcv $@ $(OBJ_OUT_FILES)

# install
# Check library directory
MAKE_INSTALL_LIB_DIR := $(wildcard $(INSTALL_LIB_DIR))

ifeq ($(strip$(MAKE_INSTALL_LIB_DIR)), )
MAKE_INSTALL_LIB_DIR = $(INSTALL_LIB_DIR)
endif

# Check include directory
MAKE_INSTALL_INC_DIR:= $(wildcard $(INSTALL_INC_DIR))

ifeq ($(strip$(MAKE_INSTALL_INC_DIR)), )
MAKE_INSTALL_INC_DIR = $(INSTALL_INC_DIR)
endif

install: $(INSTALL_INCS) $(OUTPUT_LIB)

$(OUTPUT_LIB):
	$(CP) $(BUILD_DIR)/$@ $(INSTALL_LIB_DIR)

$(INSTALL_INCS): $(MAKE_INSTALL_LIB_DIR) $(MAKE_INSTALL_INC_DIR)
	$(CP) -R $@ $(INSTALL_INC_DIR)/

$(MAKE_INSTALL_LIB_DIR):
	$(MKDIR) -p $(INSTALL_LIB_DIR)

$(MAKE_INSTALL_INC_DIR):
	$(MKDIR) -p $(INSTALL_INC_DIR)

clean: 
	rm -rf $(OBJ_OUT_FILES) $(OUTPUT_LIB) core
	rm -rf $(BUILD_DIR)

dep: 
	makedepend -- $(CFLAGS) -- $(SRCS)

