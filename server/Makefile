CXX ?= g++

.PHONY: main mr server

all: main

SCREEN_CAPTURE_LIBS = -lgdi32 -ld3d11 -ldxgi -ldwmapi
LIBS = -lscreen_capture
INCLUDE_PATH = -Iinclude/
LIBS_PATH = -Llib/

main:
	$(CXX) main.cpp -o main.exe $(INCLUDE_PATH) $(LIBS_PATH) $(LIBS) $(SCREEN_CAPTURE_LIBS)

server:
	$(CXX) server.cpp -Iinclude/ -lws2_32 -lmswsock

mr: main
	main.exe