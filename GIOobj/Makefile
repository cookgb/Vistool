# -*- Makefile -*-
###############################################################################
#
# $Id: Makefile,v 1.12 2000-07-31 15:01:42 cookgb Exp $
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
#CXXFLAGS= -O2 -DSPX
#CXXINCLUDES = /usr/ibmcxx/include

all: GIOtest

FILES= GIOtest.C
OBJS = $(FILES:.C=.o)

GIOtest: $(OBJS) libGIO.a
	$(CXX) -o GIOtest $(OBJS) libGIO.a -lm

SWAPFILES= GIOswap.C
SWAPOBJS = $(SWAPFILES:.C=.o)

GIOswap: $(SWAPOBJS) libGIO.a
	$(CXX) -o GIOswap $(SWAPOBJS) libGIO.a

LIBFILES= GIOlib.C GIObase.C
LIBOBJS= $(LIBFILES:.C=.o)
# --- FOR GNU make
libGIO.a:libGIO.a($(LIBOBJS))
# --- FOR SGI
#libGIO.a: $(LIBOBJS)
#	$(CXX) $(CXXFLAGS) -ar -o libGIO.a $(LIBOBJS)

clean:
	$(RM) $(OBJS) $(LIBOBJS) libGIO.a GIOtest

depend:
	cp -f Makefile makefile
	makedepend -f makefile -I$(CXXINCLUDES) -- $(CXXFLAGS) -- \
		$(FILES) $(LIBFILES)
