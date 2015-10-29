############################################################################
#
# Typing the command below   => results in the following being created
#      make              => newgen.o object
#      make newgen.o        => newgen.o object
#
# Object files created during the compilation process can be deleted finally
# by typing
#       make clean
#       make realclean
############################################################################

SHELL = /bin/sh

include ../../make.CHOICES

LIBDIR = ../../$(LIB_REL_DIR)
SRCDIR = ..
CHKDIR = ../..

include $(SRCDIR)/make.$(PLAT)

all: newgen.o

newgen.o:  newgen.cpp newgen.h $(SRCDIR)/sprng.h $(SRCDIR)/memory.h \
		$(SRCDIR)/primes_32.h $(SRCDIR)/store.h 
	$(CC) -c $(CFLAGS) $(INLINEOPT) newgen.cpp -I$(SRCDIR)

clean:
	rm -f *.o

realclean:
	rm -f *.o *~ core a.out
