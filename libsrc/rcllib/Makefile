.SUFFIXEDS	: .c .o

#LINUX
CC			= gcc

AR			= ar

INC         = -I../../include

DFLAG       = #-DGEN_CHK_ERR #-DCOMLIB_LOG
#CFLAGS		=  -g -Wall -O2 -finline-functions -m64 $(OPT) $(DFLAG) $(INC)
#CFLAGS		=  -g -O3 -finline-functions -m64 $(OPT) $(DFLAG) $(INC)
CFLAGS		=  -g -Wall -O3 -finline-functions -m64 $(OPT) $(DFLAG) $(INC)
#CFLAGS		=  -g -Wall -m64 $(DFLAG) $(INC)

SRCS		= rcllibInt_glob.c \
			  rcllib_api.c

OBJS		= $(SRCS:.c=.o)

TARGET_BASE = ../..

TARGET		= librcl.a
TARGET_DIR	= $(TARGET_BASE)/lib/
TARGET_INC  = $(TARGET_BASE)/include/

$(TARGET)	: $(OBJS)
		$(AR) rcv $@ $(OBJS)

all			: clean $(TARGET)

install     :
		cp $(TARGET) $(TARGET_DIR)
		cp rcllib.h $(TARGET_INC)
		cp rcllib.x $(TARGET_INC)

clean		: 
		rm -rf $(OBJS) $(TARGET) core 

dep 		: 
		makedepend -- $(CFLAGS) -- $(SRCS)

new			:
		$(MAKE) clean
		$(MAKE)
