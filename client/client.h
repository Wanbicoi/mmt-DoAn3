#include <string>
#include <vector>
#include "types.h"

int ScreenSocketGetWidth();

int ScreenSocketGetHeight();

void ScreenSocketConnect(const char *address);

void ScreenSocketGetScreen(void *data);

int ScreenSocketGetMouseInfo(int *mouse_x, int *mouse_y);

unsigned char* ScreenSocketGetMouse(int *mouse_width, int *mouse_height);

void ScreenSocketClose();

void ControlSocketConnect(const char *address);

void ControlSocketClose();

void ControlSocketSendControl(OperationCode opcode, int data = 0, void *raw_data = NULL);

// template <typename T>
// void ControlSocketSendType(T obj);

std::vector<ProcessInfo> ControlSocketGetProcesses();