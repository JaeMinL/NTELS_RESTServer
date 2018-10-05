.SUFFIXEDS	: .c .o

CC			= gcc

AR			= ar

INC         = -I../../include

DFLAG       = -DRRLLIB_LOG #-DGEN_CHK_ERR
#CFLAGS		= -g -Wall -O2 -finline-functions -ltcmalloc -m64 $(DFLAG) $(INC)
#CFLAGS		= -g -Wall -O2 -finline-functions -m64 $(DFLAG) $(INC)
CFLAGS		= -g -Wall -m64 $(DFLAG) $(INC)

SRCS		= rrllib_main.c \
			rrllib_path.c \
			rrllib_doc.c \
			rrllib_parse.c \
			rrllib_query.c \
			rrllib_mthod.c \
			rrllib_glob.c \

OBJS		= $(SRCS:.c=.o)

TARGET_BASE = ../..

TARGET		= librrl.a
TARGET_DIR	= $(TARGET_BASE)/lib/
TARGET_INC  = $(TARGET_BASE)/include/

$(TARGET)	: $(OBJS)
		$(AR) rcv $@ $(OBJS)
		cp $(TARGET) $(TARGET_DIR)

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
