# -*- Makefile -*-
###############################################################################
#
# $Id: Makefile,v 1.16 2000-07-28 17:58:30 cookgb Exp $
#
###############################################################################
# linux
#CC=gcc
#CFLAGS= -O2
#CXX=g++
#CXXFLAGS= -O2
#LIBDIRS = -L/usr/X11R6/lib GIO/libGIO.a

# SGI
CC=cc -mips4
CFLAGS = -O2
CXX=CC -mips4
CXXFLAGS= -O2
LIBDIRS = GIO/libGIO.a

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

X11LIBS= $(LIBDIRS) -lGLw -lXm -lXt -lGL -lGLU -lX11

xvistool: $(FILES)
	$(CXX) -o xvistool $(FILES) $(X11LIBS) -lm

xvistool_sgi: $(FILES)
	$(CXX) -o xvistool $(FILES) -lSgm $(X11LIBS) -lm

