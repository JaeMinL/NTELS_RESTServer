.SUFFIXEDS	: .c .o

CC			= gcc

AR			= ar

INC         = -I../../include

DFLAG       = -DSMULIB_LOG #-DGEN_CHK_ERR

CFLAGS		= -g -Wall -O2 -finline-functions -m64 $(DFLAG) $(INC)
#CFLAGS		= -g -Wall -m64 $(DFLAG) $(INC)

SRCS		= smulibInt_glob.c\
			  smulibInt_init.c\
			  smulibInt_lh.c\
			  smulibInt_main.c\
			  smulibInt_sig.c\
			  smulibInt_thBlk.c\
			  smulibInt_timer.c\
			  smulibInt_uh.c\
			  smulib_api.c

OBJS		= $(SRCS:.c=.o)

TARGET_BASE = ../..

TARGET		= libsmu.a
TARGET_DIR	= $(TARGET_BASE)/lib/
TARGET_INC  = $(TARGET_BASE)/include/

$(TARGET)	: $(OBJS)
		$(AR) rcv $@ $(OBJS)

all			: clean $(TARGET)

install     :
		cp $(TARGET) $(TARGET_DIR)
		cp smulib.h $(TARGET_INC)
		cp smulib.x $(TARGET_INC)

clean		: 
		rm -rf $(OBJS) $(TARGET) core 

dep 		: 
		makedepend -- $(CFLAGS) -- $(SRCS)

new			:
		$(MAKE) clean
		$(MAKE)
