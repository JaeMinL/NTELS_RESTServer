.SUFFIXEDS	: .c .o

#LINUX
CC			= gcc

AR			= ar

INC         = -I../../include

DFLAG       = -DRLYLIB_LOG #-DGEN_CHK_ERR

CFLAGS		= -Wall -g -m64 -O2 -finline-functions $(DFLAG) $(INC)
#CFLAGS		= -Wall -g -m64 $(DFLAG) $(INC)

SRCS		= rlylib_api.c \
			  rlylibInt_acpt.c \
			  rlylibInt_glob.c \
			  rlylibInt_host.c \
			  rlylibInt_init.c \
			  rlylibInt_dstry.c \
			  rlylibInt_msg.c \
			  rlylibInt_rlm.c \
			  rlylibInt_sync.c \

OBJS		= $(SRCS:.c=.o)

TARGET_BASE = ../..

TARGET		= librly.a
TARGET_DIR	= $(TARGET_BASE)/lib/
TARGET_INC  = $(TARGET_BASE)/include/

$(TARGET)	: $(OBJS)
		$(AR) rcv $@ $(OBJS)

all			: clean $(TARGET)

install     :
		cp $(TARGET) $(TARGET_DIR)
		cp rlylib.h $(TARGET_INC)
		cp rlylib.x $(TARGET_INC)

clean		: 
		rm -rf $(OBJS) $(TARGET) core 

dep 		: 
		makedepend -- $(CFLAGS) -- $(SRCS)

new			:
		$(MAKE) clean
		$(MAKE)
