# -*- Makefile -*-
CXX=CC
CXXFLAGS= -O2

all: xvistool

xvistool.o: xvistool.h vistool.h
xvistool.h: vistool.h
vistool.o: vistool.h

FILES= xvistool.o vistool.o

X11LIBS= -lXm -lXt -lX11

xvistool: $(FILES)
	$(CXX) -o xvistool $(FILES) $(X11LIBS)

