# -*- Makefile -*-
###############################################################################
#
# $Id: Makefile,v 1.4 1999-12-30 17:03:12 cookgb Exp $
#
###############################################################################
# linux
#CC=gcc
#CFLAGS= -O2
#CXX=g++
#CXXFLAGS= -O2 -fexternal-templates
#LIBDIRS = -L/usr/X11R6/lib

# SGI
CC=cc -mips4
CFLAGS = -O2
CXX=CC -mips4
CXXFLAGS= -O2 -LANG:bool=OFF

# SP2
#CC=xlc
#CFLAGS = -O2
#CXX=xlC
#CXXFLAGS= -O2

all: GIOtest

GIObase.o: GIOobj.h
GIOobj.o: GIOobj.h
GIO1d.o: GIO1d.h GIOobj.h
GIOtest.o: GIO1d.h GIOobj.h

FILES= GIOtest.o GIO1d.o GIOobj.o GIObase.o

GIOtest: $(FILES)
	$(CXX) -o GIOtest $(FILES) -lm


