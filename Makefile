#
# Makefile for libNASL
# Chris Olsen
#
CC = gcc -I. -Wall -pedantic -ansi
OBJS = shader.o shaderLex.o shaderParse.o textureManager.o nasl.o
BIN = ../bin/libnasl.so
LIBS = -L/usr/X11R6/lib -lGL -lGLU -lXxf86vm
VPATH = obj/:


all: $(BIN)

$(BIN): $(OBJS)
	if [[ ! -e obj ]]; then mkdir obj; fi; if [[ ! -e bin ]]; then mkdir bin; fi;  cd obj/; $(CXX) $(OBJS) -fPIC -shared -o ../bin/libnasl.so


parser/shaderLex.cpp parser/shaderParse.cpp: parser/shader.l parser/shader.y
	flex  -Pshader -oparser/shaderLex.cpp parser/shader.l
	bison -d -pshader -o parser/shaderParse.cpp parser/shader.y
	mv parser/shaderParse.hpp parser/shaderParse.cpp.h

%.o: %.cpp
	if [[ ! -e obj ]]; then mkdir obj; fi; $(CXX) $(CXXFLAGS) -c $< -o obj/$@

%.o: parser/%.cpp
	if [[ ! -e obj ]]; then mkdir obj; fi; $(CXX) $(CXXFLAGS) -c $< -o obj/$@

install: $(BIN)
	cp bin/libnasl.so /usr/lib/; cp nasl.h /usr/include/

doxygen:
	doxygen libnasl.cfg

clean:
	@echo Cleaning...; rm obj/*.o; rm parser/shaderParse.cpp*; rm parser/shaderLex.cpp; rm bin/libnasl.so; rm `find -name *.~`
