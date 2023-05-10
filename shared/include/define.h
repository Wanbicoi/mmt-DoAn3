//#define UDP_SOCKET_PORT 5445
//#define TCP_SOCKET_PORT 4435
#pragma once

#define SOCKET_SCREEN_PORT 5343
#define SOCKET_CONTROL_PORT 4354

enum OperationCode : uint16_t {
	NONE = 0,
	APP_START,
	APP_STOP,
	PROCESS_LIST,
	PROCESS_START,
	PROCESS_STOP,
	KEYBOARD_DOWN,
	MOUSE_LEFT,
	MOUSE_MIDDLE,
	MOUSE_RIGHT,
	FS_LIST,
	FS_COPY,
	FS_ADD,
	FS_DELETE
};