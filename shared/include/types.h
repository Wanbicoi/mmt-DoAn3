//#define UDP_SOCKET_PORT 5445
//#define TCP_SOCKET_PORT 4435
#pragma once
#include <string>

#define SOCKET_SCREEN_PORT 5343
#define SOCKET_CONTROL_PORT 4354

enum OperationCode : uint16_t {
	NONE = 0,
	SCREEN_CONNECT,
	//Processes and Applications
	PROCESS_LIST,
	PROCESS_SUSPEND,
	PROCESS_RESUME,
	PROCESS_KILL,
	//Input: Keyboard
	KEYBOARD_DOWN,
	//Input: Mouse
	MOUSE_MOVE,
	MOUSE_LEFT_DOWN,
	MOUSE_LEFT_UP,
	MOUSE_MIDDLE_DOWN,
	MOUSE_MIDDLE_UP,
	MOUSE_RIGHT_DOWN,
	MOUSE_RIGHT_UP,
	MOUSE_WHEEL_V,
	MOUSE_WHEEL_H,
	//File system
	FS_INIT,
	FS_LIST,
	FS_COPY,
	FS_MOVE,
	FS_WRITE,
	FS_ASK_OPTION,
	FS_DELETE
};

struct ScreenInfo {
	int width;
	int height;
};

struct MousePosition {
	float x;
	float y;
	int width;
	int height;
};

struct FrameBuffer {
	int mouse_x;
	int mouse_y;
	int mouse_changed;
	int mouse_width;
	int mouse_height;
	int mouse_center_x;
	int mouse_center_y;
	int mouse_size;
	void *mouse_data;
	int screen_size;
	void *screen_data;
};

struct ProcessInfo {
	int pid;
	std::string name;
	char type;
};

struct FileInfo {
	std::string name;
	char type;
};