# -*- Makefile -*-
###############################################################################
#
# $Id: Makefile,v 1.18 2000-08-05 21:58:36 cookgb Exp $
#
###############################################################################
# linux
CC=gcc
CFLAGS= -O2
CXX=g++
CXXFLAGS= -O2
LIBDIRS = -L/usr/X11R6/lib GIO/libGIO.a
X11LIBS= $(LIBDIRS) -lGLw -lXm -lXt -lGL -lGLU -lX11

# SGI
#CC=cc -mips4
#CFLAGS = -O2
#CXX=CC -mips4
#CXXFLAGS= -O2
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

xvistool: $(FILES)
	$(CXX) -o xvistool $(FILES) $(X11LIBS) $(CXXFLAGS) -lm

xvistool_sgi: $(FILES)
	$(CXX) -o xvistool $(FILES) -lSgm $(X11LIBS) -lm

