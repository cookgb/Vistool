# -*- Makefile -*-
###############################################################################
#
# $Id: Makefile,v 1.22 2001-06-12 14:45:19 cookgb Exp $
#
###############################################################################
# linux
#%%%CC=gcc
#%%%CFLAGS= -O2
#%%%CXX=g++
#%%%CXXFLAGS= -O2
#%%%CXXINCLUDES = /usr/include/g++-2

# SGI
CC=cc -mips4
CFLAGS = -O2
CXX=CC -mips4
CXXFLAGS= -O2 -LANG:bool=OFF
GIOARFLAGS= -ar
CXXINCLUDES = /usr/include/CC
TEMPLATE_DIR = ii_files

# SP2
#%%%CC=xlc
#%%%CFLAGS = -O2
#%%%CXX=xlC
#%%%CXXFLAGS= -O2 -DSPX
#%%%CXXINCLUDES = /usr/ibmcxx/include
#%%%TEMPLATE_DIR = tempinc

# SP2 with KCC
#%%%CC=xlc
#%%%CFLAGS = -O2
#%%%CXX=KCC
#%%%CXXFLAGS= +K2 -DKCC -qmaxmem=4048
#%%%CXXINCLUDES = /pub/share/KAICPP/KCC_BASE/include
#%%%TEMPLATE_DIR = ti_files *.ti

# SUN with gcc
#%%%CC=gcc
#%%%CFLAGS= -O2
#%%%CCC=g++
#%%%CXX=g++
#%%%CXXFLAGS= -O2
#%%%CXXINCLUDES = /usr/local/include/g++-3

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
#libGIO.a:libGIO.a($(LIBOBJS))
# --- FOR SGI and KCC
libGIO.a: $(LIBOBJS)
	$(CXX) $(CXXFLAGS) $(GIOARFLAGS) -o $@ $(LIBOBJS)

clean:
	$(RM) $(OBJS) $(LIBOBJS) $(SWAPOBJS) libGIO.a GIOtest GIOswap
	$(RM) -r $(TEMPLATE_DIR)

depend:
	cp -f Makefile makefile
	make localdepend

localdepend:
	makedepend -f makefile \
		-- -I$(CXXINCLUDES) $(CXXFLAGS) -- \
		$(FILES) $(LIBFILES)
# DO NOT DELETE
