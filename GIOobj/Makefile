# -*- Makefile -*-
###############################################################################
#
# $Id: Makefile,v 1.2 1999-12-30 14:35:59 cookgb Exp $
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
CXXFLAGS= -O2 -LANG:ansi-for-init-scope=ON

all: GIOtest

GIOobj.o: GIOobj.h
GIO1d.o: GIO1d.h GIOobj.h
GIOtest.o: GIO1d.h GIOobj.h

FILES= GIOtest.o GIO1d.o GIOobj.o

GIOtest: $(FILES)
	$(CXX) -o GIOtest $(FILES) -lm


