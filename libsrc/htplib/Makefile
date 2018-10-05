.SUFFIXEDS	: .c .o

#LINUX
CC			= gcc

AR			= ar

INC         = -I../../include

#DFLAG       = -DHTPLIB_LOG
#CFLAGS		=  -g -Wall -O2 -finline-functions -m64 $(OPT) $(DFLAG) $(INC)
#CFLAGS		=  -g -Wall -O3 -finline-functions -m64 $(OPT) $(DFLAG) $(INC)
CFLAGS		=  -Wall -O3 -finline-functions -m64 $(OPT) $(DFLAG) $(INC)
#CFLAGS		=  -g -Wall -finline-functions -m64 $(OPT) $(DFLAG) $(INC)
#CFLAGS		=  -g -Wall -m64 $(DFLAG) $(INC)

SRCS		= htplib_init.c \
			  htplib_main.c \
			  htplib_staCode.c \
			  htplib_mthod.c \

OBJS		= $(SRCS:.c=.o)

TARGET_BASE = ../..

TARGET		= libhtp.a
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
