.SUFFIXEDS	: .c .o

CC			= gcc

AR			= ar

INC         = -I../../include

DFLAG       = #-DTHRLIB_LOG -DGEN_CHK_ERR

CFLAGS		= -Wall -g -m64 -O2 -finline-functions $(DFLAG) $(INC)
#CFLAGS		= -Wall -g -pg -m64 $(DFLAG) $(INC)

SRCS		= thrlib_thrd.c \
			  thrlib_mutx.c \
			  thrlib_cond.c \
			  thrlib_tq.c \
			  thrlib_pool.c \

OBJS		= $(SRCS:.c=.o)

TARGET_BASE = ../..

TARGET		= libthr.a
TARGET_DIR	= $(TARGET_BASE)/lib/
TARGET_INC  = $(TARGET_BASE)/include/

$(TARGET)	: $(OBJS)
		$(AR) rcv $@ $(OBJS)

all			: clean $(TARGET)

install     :
	    mkdir -p $(TARGET_DIR)
		mkdir -p $(TARGET_INC)
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
