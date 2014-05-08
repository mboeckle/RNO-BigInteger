# RNO: Team <9>
# Anzengruber Philipp <0631149>
# Boeckle Martin <0630994>
# Stumpfl Tobias <0530501>
#-------------------------------
# BigInteger.cpp
#
# description:
#	Makefile for Assignment1: Compile and Link C/C++ source
#
# changes:
#	20070311-jwolkers:	initial version

TITLE = Assignment_A1

OBJS := $(patsubst %.cpp,%.o,$(wildcard *.cpp))
CC = g++
LD = g++ 
CC_FLAGS = -c -g -m32
EXECUTABLE = $(TITLE)
LD_FLAGS = -m32 -o $(EXECUTABLE)

$(EXECUTABLE) : $(OBJS)
	$(LD) $(LD_FLAGS) $(OBJS)

%.o: %.cpp *.h
	$(CC) $(CC_FLAGS) $<

$(TITLE).zip : *.h *.cpp Makefile
	zip $@ $^

all : $(EXECUTABLE)

run : $(EXECUTABLE)
	./$^

dbg : $(EXECUTABLE)
	ddd ./$^

dist : $(TITLE).zip
	@echo
	@echo "Online-Abgabe von '$<': http://www.iaik.tugraz.at/teaching/03_rechnernetze%20und%20organisation/"

clean : 
	rm *.o $(EXECUTABLE) $(TITLE).zip *.~*~ core* 
