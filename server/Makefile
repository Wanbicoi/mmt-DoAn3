CXX ?= g++

.PHONY: main mr server

all: main


main:
	$(CXX) main.cpp -o main.exe

server:
	$(CXX) server.cpp -Iinclude/ -lws2_32 -lmswsock

mr: main
	main.exe