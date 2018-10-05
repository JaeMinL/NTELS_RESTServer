.SUFFIXEDS	: .c .o

#LINUX
CC			= gcc

AR			= ar

INC         = -I../../include

DFLAG       = #-DGEN_CHK_ERR #-DCOMLIB_LOG
CFLAGS		=  -g -Wall -O2 -finline-functions -m64 $(OPT) $(DFLAG) $(INC)

#SRCS		= 

OBJS		= $(SRCS:.c=.o)

TARGET_BASE = ../..

#TARGET		= libcom.a
TARGET_DIR	= $(TARGET_BASE)/lib/
TARGET_INC  = $(TARGET_BASE)/include/

$(TARGET)	: $(OBJS)
		$(AR) rcv $@ $(OBJS)

all			: 
	    mkdir -p $(TARGET_DIR)
		mkdir -p $(TARGET_INC)
		cp *.h $(TARGET_INC)
		cp *.x $(TARGET_INC)

clean		: 
		rm -rf $(OBJS) $(TARGET) core 

dep 		: 
		makedepend -- $(CFLAGS) -- $(SRCS)

new			:
		$(MAKE) clean
		$(MAKE)
