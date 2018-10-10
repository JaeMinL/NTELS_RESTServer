#
## Makefile to compile source directories
#
#
#SHELL  			= /bin/sh

MAKE   			= make
RM              = rm -rf

SOURCES			= genhdr\
   				  comlib\
   				  thrlib\
   				  trnlib\
   				  rlylib\
   				  rrllib\
   				  loglib\
   				  htplib\
   				  smulib\
   				  wldlib\
   				  rsvlib\
   				  rcllib\

all::
	       	@for i in $(SOURCES) ; \
			do \
				(cd $$i ; echo "making all in ./$$i" ; \
					$(MAKE); $(MAKE) install;) \
			done

clean::
	       	@for i in $(SOURCES) ; \
				do \
				(cd $$i ; echo "cleaning all in ./$$i" ; \
				$(MAKE) clean; )\
				done\

