# Makefile pour OpenCV
#
# CC BY-SA Edouard.Thiel@univ-amu.fr - 07/09/2020

# Pour installer OpenCV sous Linux, taper :
#  sudo apt-get install libopencv-dev
shell export PKG_CONFIG_PATH="/usr/local/opt/opencv@2/lib/pkgconfig"
UNAME := $(shell uname)
SHELL	= /bin/bash
CC      = g++
RM      = rm -f
ifeq ($(UNAME), Darwin)
CFLAGS  = -Wall --std=c++14 $$(pkg-config opencv --cflags)
LIBS    = $$(pkg-config opencv --libs)
else
CFLAGS  = -Wall --std=c++14 $$(pkg-config opencv --cflags)
LIBS    = $$(pkg-config opencv --libs)
endif

CFILES  := $(wildcard *.cpp)
CFILES  := $(filter-out gd-util.cpp, $(CFILES))
EXECS   := $(CFILES:%.cpp=%)

%.o : %.cpp
	$(CC) $(CFLAGS) -c $*.cpp

all :: $(EXECS)

$(EXECS) : % : %.o gd-util.o
	$(CC) -o $@ $^ $(LIBS)

clean ::
	$(RM) *.o *~ $(EXECS) tmp*.*


