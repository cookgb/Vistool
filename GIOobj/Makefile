# -*- Makefile -*-
###############################################################################
#
# $Id: Makefile,v 1.7 2000-01-03 13:41:01 cookgb Exp $
#
###############################################################################
# linux
CC=gcc
CFLAGS= -O2
CXX=g++
CXXFLAGS= -O2
CXXINCLUDES = /usr/include/g++-2

# SGI
#CC=cc -mips4
#CFLAGS = -O2
#CXX=CC -mips4
#CXXFLAGS= -O2 -LANG:bool=OFF
#CXXINCLUDES = /usr/include/CC

# SP2
#CC=xlc
#CFLAGS = -O2
#CXX=xlC
#CXXFLAGS= -O2
#CXXINCLUDES = /usr/ibmcxx/include

all: GIOtest

FILES= GIOtest.C
OBJS = $(FILES:.C=.o)

GIOtest: $(OBJS) libGIO.a
	$(CXX) -o GIOtest $(OBJS) libGIO.a -lm

LIBFILES= GIOlib.C GIObase.C
LIBOBJS= $(LIBFILES:.C=.o)
libGIO.a:libGIO.a($(LIBOBJS))

clean:
	$(RM) $(OBJS) $(LIBOBJS) libGIO.a GIOtest

depend:
	cp -f Makefile makefile
	makedepend -f makefile -I$(CXXINCLUDES) -- $(CXXFLAGS) -- \
		$(FILES) $(LIBFILES)
