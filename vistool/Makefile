# -*- Makefile -*-
###############################################################################
#
# $Id: Makefile,v 1.4 1999-12-23 18:20:17 cookgb Exp $
#
###############################################################################
# linux
#CXX=g++
#CXXFLAGS= -O2
#LIBDIRS = -L/usr/X11R6/lib

# SGI
CXX=CC
CXXFLAGS= -O2

all: xvistool

xvistool.o: xvistool.h vistool.h sovLayerUtil.h
xvistool.h: vistool.h
vistool.o: vistool.h
sovlayerutil.o: sovLayerUtil.h

FILES= xvistool.o vistool.o sovlayerutil.o

X11LIBS= $(LIBDIRS) -lGLw -lXm -lXt -lGL -lGLU -lX11

xvistool: $(FILES)
	$(CXX) -o xvistool $(FILES) $(X11LIBS)

