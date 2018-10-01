.SUFFIXEDS	: .c .o

CC			= gcc

AR			= ar

INC         = -I../../include


DFLAG       = #-DTRNLIB_LOG -DGEN_CHK_ERR

#CFLAGS		= -Wall -g -m64 -O2 -finline-functions $(DFLAG) $(INC)
CFLAGS		= -Wall -m64 -O3 -finline-functions $(DFLAG) $(INC)
#CFLAGS		= -Wall -g -m64 $(DFLAG) $(INC)

SRCS		= trnlib_sock.c \
			  trnlib_util.c \

OBJS		= $(SRCS:.c=.o)

TARGET_BASE = ../..

TARGET		= libtrn.a
TARGET_DIR	= $(TARGET_BASE)/lib/
TARGET_INC  = $(TARGET_BASE)/include/

$(TARGET)	: $(OBJS)
		$(AR) rcv $@ $(OBJS)

all			: clean $(TARGET)

install     :
		cp $(TARGET) $(TARGET_DIR)
		cp *.h $(TARGET_INC)
		cp *.x $(TARGET_INC)

clean		: 
		rm -rf $(OBJS) $(TARGET) core 

dep 		: 
		makedepend -- $(CFLAGS) -- $(SRCS)

new			:
		$(MAKE) clean
		$(MAKE)
