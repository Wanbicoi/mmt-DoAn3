//#define UDP_SOCKET_PORT 5445
//#define TCP_SOCKET_PORT 4435
#pragma once
#include <string>
#include <vector>

#define SOCKET_SCREEN_PORT 5343
#define SOCKET_CONTROL_PORT 4354

enum OperationCode : uint16_t {
	NONE = 0,
	SCREEN_CONNECT,
	FRAME_DATA,
	CONTROL_DISCONNECT,
	//Processes and Applications
	PROCESS_LIST,
	PROCESS_SUSPEND,
	PROCESS_RESUME,
	PROCESS_KILL,
	//File system
	FS_INIT,
	FS_LIST,
	FS_CHECK_EXIST,
	FS_COPY,
	FS_MOVE,
	FS_WRITE,
	FS_DELETE
};

struct ScreenInfo {
	int width;
	int height;
};

struct FrameBuffer {
	bool keys[256];
	int mouse_x;
	int mouse_y;
	int mouse_changed;
	int mouse_width;
	int mouse_height;
	int mouse_center_x;
	int mouse_center_y;
	int mouse_size;
	void *mouse_data;
	int screen_changed;
	int screen_size;
	void *screen_data;
};

enum FileEntry : char {
	ENTRY_FILE,
	ENTRY_FOLDER,
	ENTRY_PARENT,
	ENTRY_DRIVE
};

struct ProcessInfo {
	int pid;
	std::string name;
	char type;
};

struct FileInfo {
	std::string name;
	FileEntry type;
};