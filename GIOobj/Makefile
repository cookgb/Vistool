# -*- Makefile -*-
###############################################################################
#
# $Id: Makefile,v 1.16 2000-08-10 14:28:52 cookgb Exp $
#
###############################################################################
# linux
#CC=gcc
#CFLAGS= -O2
#CXX=g++
#CXXFLAGS= -O2
#CXXINCLUDES = /usr/include/g++-2

# SGI
#CC=cc -mips4
#CFLAGS = -O2
#CXX=CC -mips4
#CXXFLAGS= -O2 -LANG:bool=OFF
#CXXINCLUDES = /usr/include/CC
#TEMPLATE_DIR = ii_files

# SP2
CC=xlc
CFLAGS = -O2
CXX=xlC
CXXFLAGS= -O2 -DSPX
CXXINCLUDES = /usr/ibmcxx/include
TEMPLATE_DIR = tempinc

all: GIOtest GIOswap

FILES= GIOtest.C
OBJS = $(FILES:.C=.o)

GIOtest: $(OBJS) libGIO.a
	$(CXX) -o GIOtest $(OBJS) libGIO.a -lm

SWAPFILES= GIOswap.C
SWAPOBJS = $(SWAPFILES:.C=.o)

GIOswap: $(SWAPOBJS) libGIO.a
	$(CXX) -o GIOswap $(SWAPOBJS)

LIBFILES= GIOlib.C GIObase.C
LIBOBJS= $(LIBFILES:.C=.o)
# --- FOR GNU make
libGIO.a:libGIO.a($(LIBOBJS))
# --- FOR SGI
#libGIO.a: $(LIBOBJS)
#	$(CXX) $(CXXFLAGS) -ar -o libGIO.a $(LIBOBJS)

clean:
	$(RM) $(OBJS) $(LIBOBJS) $(SWAPOBJS) libGIO.a GIOtest GIOswap
	$(RM) -r $(TEMPLATE_DIR)

depend:
	cp -f Makefile makefile
	makedepend -f makefile -I$(CXXINCLUDES) -- $(CXXFLAGS) -- \
		$(FILES) $(LIBFILES)
