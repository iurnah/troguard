CC=gcc
CFLAGS=-I.
DEPS=trace_parser.h
OBJ=app_profile.o

.PHONY: all
all: app_profile

app_profile: app_profile.cpp trace_parser.cpp
	g++ -o app_profile app_profile.cpp trace_parser.cpp
	
.PHONY: install
install:

.PHONY: clean
clean:
	rm -f hello
