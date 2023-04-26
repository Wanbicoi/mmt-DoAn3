CXX ?= g++

.PHONY: main mr

all: main

RAYLIB_FLAG = -Iinclude/ -Llib/ -lraylib -lopengl32 -lgdi32 -lwinmm
NO_CONSOLE = -mwindows

main:
	$(CXX) main.cpp $(RAYLIB_FLAG) -o main.exe

mr: main
	main.exe