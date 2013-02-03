SOURCES = $(shell find ./ -name "*.cpp")
DEBUG_OBJECTS = $(SOURCES:.cpp=.d)
OBJECTS = $(SOURCES:.cpp=.o)
OUTPUT = sim_linux

CC = g++
CFLAGS = -fms-extensions -std=c++0x -g
INCLUDES = -I./ -I../extlibs/include -I/usr/local/include/assimp
MACROS = -DPLATFORM_LINUX
DEBUG_MACROS = -D_DEBUG
RELEASE_MACROS = -DNO_PROFILE -DNO_CHECK_MACROS -DNDEBUG 
INTERMEDIATE = ./intermediate

LD = g++
LDFLAGS = 
LIBS = -lGL -lGLU -lGLEW -lX11 -lIL -lILU -lassimp 

default: all

clean:
	find ./ -name "*.o" -exec rm {} \;
	if [ -e $(OUTPUT) ]; then rm $(OUTPUT); fi

all: debug release

rebuild: clean all

%.d: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $(DEBUG_MACROS) $(MACROS) -c -o $@ $<

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $(RELEASE_MACROS) $(MACROS) -c -o $@ $<

debug: $(DEBUG_OBJECTS)
	$(LD) $(LDFLAGS) -o $(OUTPUT)_debug $(DEBUG_OBJECTS) $(LIBS) 
	cp ./sim_linux_debug ../bin/

release: $(OBJECTS)
	$(LD) $(LDFLAGS) -o $(OUTPUT)_release $(OBJECTS) $(LIBS) 
	cp ./sim_linux_release ../bin/

