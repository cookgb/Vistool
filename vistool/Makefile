# -*- Makefile -*-
###############################################################################
#
# $Id: Makefile,v 1.25 2002-07-26 17:58:09 cookgb Exp $
#
###############################################################################
# linux/gcc3
CC=gcc3
CFLAGS= -O2
CXX=g++3
CXXFLAGS= -O2 -I/usr/X11R6/LessTif/Motif1.2/include -I/usr/X11R6/include
LIBDIRS = -L/usr/X11R6/LessTif/Motif1.2/lib -L/usr/X11R6/lib GIO/libGIO.a
X11LIBS= $(LIBDIRS) -lGLw -lXm -lXt -lGL -lGLU -lX11

# linux/Intel
#CC=icc
#CFLAGS= -O2
#CXX=icpc
#CXXFLAGS= -O2 -I/usr/X11R6/LessTif/Motif1.2/include -I/usr/X11R6/include
#LIBDIRS = -L/usr/X11R6/LessTif/Motif1.2/lib -L/usr/X11R6/lib GIO/libGIO.a
#X11LIBS= $(LIBDIRS) -lGLw -lXm -lXt -lGL -lGLU -lX11

# SGI
#CC=cc -mips4
#CFLAGS = -O2
#CXX=CC -mips4
#CXXFLAGS= -O2 -LANG:std -DSGI
#LIBDIRS = GIO/libGIO.a
#X11LIBS= $(LIBDIRS) -lGLw -lXm -lXt -lGL -lGLU -lX11

# SP2
#CC=xlc
#CFLAGS = -O2
#CXX=xlC
#CPPFLAGS=
#CXXFLAGS= -DSPX -O2 -w -I/pub/share/include/STLport-4.0/stlport
#LIBDIRS = GIO/libGIO.a
#CXXINCLUDES = /usr/ibmcxx/include
#X11LIBS= $(LIBDIRS) -lGLw -lXm -lXt -lGL -lGLU -lX11 -brtl

# SUN with gcc
#CC=gcc
#CFLAGS= -O2
#CCC=g++
#CXX=$(CCC)
#CCFLAGS= -O2 -I/usr/openwin/include/GL -I/home/supernova/scheel
#CXXFLAGS= $(CCFLAGS)
#LIBDIRS = -L/usr/openwin/lib GIO/libGIO.a
#X11LIBS= $(LIBDIRS) -lGLw -lXm -lXt -lGL -lGLU -lX11

all: xvistool

xvistool.o: xvistool.h vistool.h xMenu.h sovLayerUtil.h
xvistool.h: vistool.h
vistool.o: vistool.h
xobjectinterface.o: xvistool.h vistool.h
xfuncinterface.o: xvistool.h vistool.h
xMenu.o: xMenu.h
sovlayerutil.o: sovLayerUtil.h

FILES= xvistool.o vistool.o xobjectinterface.o xfuncinterface.o xMenu.o \
       sovlayerutil.o
## VendorS.o

xvistool: $(FILES)
	$(CXX) -o xvistool $(CXXFLAGS) $(FILES) $(X11LIBS) -lm

xvistool_sgi: $(FILES)
	$(CXX) -o xvistool $(CXXFLAGS) $(FILES) -lSgm $(X11LIBS) -lm

