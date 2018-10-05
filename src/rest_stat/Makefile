#!/bin/sh

#LINUX
CC				= gcc

RM				= rm -f

BASE_HOME = ../..
SRC_HOME  = .

OUT_DIR   = .
DST_DIR   = $(BASE_HOME)/bin
INC_DIR   = $(BASE_HOME)/include
LIB_DIR   = $(BASE_HOME)/lib

OBJ       = o

#Fixed System Flag 
###################################
#              LIST               #
###################################
SYS_FIXED_FLAG  = AUTO

ENV = -Wall  -DLOGLIB_DISP_ENV_LVL -DLOGLIB_DISP_ENV_FILE -DLOGLIB_DISP_ENV_LINE -DLOGLIB_DISP_ENV_TIME

COPTS     = -g  -lpthread -luriparser -lxml2 -lmicrohttpd -lmysqlclient -ljson -ljson-c
#IOPTS     = -I$(INC_DIR) -I/usr/include/libxml2 -L/usr/lib64/mysql
#IOPTS     = -I$(INC_DIR) -I/usr/include/libxml2 -L/usr/lib/ -L/usr/lib64 -L/usr/lib/mysql
IOPTS     = -I$(INC_DIR) -I/usr/include/libxml2 -L/usr/lib/ -L/usr/lib64/mysql -L/usr/lib/mysql -L/usr/include/json -L/usr/include/json-c

POPTS     = $(DEBUG) $(DEBUG_TRC)

LIBS      = \
		$(BASE_HOME)/lib/librsv.a\
		$(BASE_HOME)/lib/librrl.a\
		$(BASE_HOME)/lib/librly.a\
		$(BASE_HOME)/lib/libtrn.a\
		$(BASE_HOME)/lib/liblog.a\
		$(BASE_HOME)/lib/libthr.a\
		$(BASE_HOME)/lib/libcom.a\
		$(BASE_HOME)/lib/libtoml.a\


SRCS		= main.c change_date.c

OBJS			= $(SRCS:.c=.o)

AOUT			= rest_stat

DEST			= ./

#
#---------------------------------------------------------------------
#
all:: $(AOUT) install

.SUFFIXES: .c .o
.c.o:
		$(CC) $(COPTS) $(ENV) $(LOPTS) $(IOPTS) $(POPTS) -c $<;

$(AOUT): $(OBJS)
		$(CC) $(COPTS) $(ENV) $(LOPTS) -o $(AOUT) $(IOPTS) $(POPTS) $(OBJS) $(LIBS);\

install:
	cp $(AOUT) ../../bin

clean:
	\rm -f $(OBJS) $(AOUT) core ;\
