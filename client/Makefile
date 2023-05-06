CXX ?= g++

.PHONY: main mr client

all: main

RAYLIB_FLAG = -Iinclude/ -Llib/ -lraylib -lopengl32 -lgdi32 -lwinmm
NO_CONSOLE = -mwindows

main:
	$(CXX) main.cpp $(RAYLIB_FLAG) -o main.exe

client:
	$(CXX) client.cpp -Iinclude/ -lws2_32 -lmswsock

mr: main
	main.exe