# this is the name of the output executable
TARGET = log_parser

# these are the compile time flags passed to gcc
CFLAGS = -ggdb  -DDEBUG -DDLIB_NO_GUI_SUPPORT  -I ./dlib-17.28  -Wall

# These are the link time flags passed to gcc
LFLAGS = -lpthread -lnsl 

# The name of the compiler.
CC = g++

####################################################
####################################################
#  Here we list all the cpp files we want to compile

SRC = log_parser.cpp
SRC += ./dlib-17.28/dlib/all/source.cpp 

####################################################

TMP = $(SRC:.cpp=.o)
OBJ = $(TMP:.c=.o)

all: $(TARGET) 

$(TARGET): $(OBJ) 
	@echo Linking $@
	@$(CC) $(LFLAGS) $(OBJ) -o $@ -lpthread
	@echo Build Complete

.cpp.o: $<
	@echo Compiling $<
	@$(CC) -c $(CFLAGS) $< -o $@

clean:
	@rm -f $(OBJ) $(TARGET)
	@echo All object files and binaries removed

dep: 
	@echo Running makedepend
	@makedepend -- $(CFLAGS) -- $(SRC) 2> /dev/null 
	@echo Completed makedepend

