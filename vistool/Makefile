# -*- Makefile -*-
###############################################################################
#
# $Id: Makefile,v 1.3 1999-10-28 19:07:19 gcook Exp $
#
###############################################################################

CXX=CC
CXXFLAGS= -O2

all: xvistool

xvistool.o: xvistool.h vistool.h sovLayerUtil.h
xvistool.h: vistool.h
vistool.o: vistool.h
sovlayerutil.o: sovLayerUtil.h

FILES= xvistool.o vistool.o sovlayerutil.o

X11LIBS= -lGLw -lXm -lXt -lGL -lGLU -lX11

xvistool: $(FILES)
	$(CXX) -o xvistool $(FILES) $(X11LIBS)

