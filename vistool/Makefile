# -*- Makefile -*-
###############################################################################
#
# $Id: Makefile,v 1.15 2000-03-27 13:09:29 cookgb Exp $
#
###############################################################################
# linux
CC=gcc
CFLAGS= -O2
CXX=g++
CXXFLAGS= -O2
LIBDIRS = -L/usr/X11R6/lib GIO/libGIO.a

# SGI
#CC=cc -mips4
#CFLAGS = -O2
#CXX=CC -mips4
#CXXFLAGS= -O2
#LIBDIRS = GIO/libGIO.a

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
	$(CXX) -o xvistool $(FILES) $(X11LIBS)

xvistool_sgi: $(FILES)
	$(CXX) -o xvistool $(FILES) -lSgm $(X11LIBS)

