#
### Makefile to compile source directories
#
#
#SHELL              = /bin/sh

MAKE            = gmake
RM              = rm -rf

SOURCES         = libsrc\
                  src\

all::
	@for i in $(SOURCES) ; \
	do \
		(cd $$i ; echo "making all in ./$$i" ; \
			$(MAKE) all;) \
	done


clean::
	@for i in $(SOURCES) ; \
	do \
		(cd $$i ; echo "cleaning all in ./$$i" ; \
			$(MAKE) clean; )\
	done\

install::
