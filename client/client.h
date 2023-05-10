#include <string>
#include <vector>
#include <tuple>

int ScreenSocketGetWidth();

int ScreenSocketGetHeight();

void ScreenSocketConnect(const char *address);

void ScreenSocketGetScreen(void *data);

int ScreenSocketGetMouseInfo(int &mouse_x, int &mouse_y);

unsigned char* ScreenSocketGetMouse(int &mouse_width, int &mouse_height);

void ScreenSocketClose();

void ControlSocketConnect(const char *address);

void ControlSocketClose();

void ControlSocketSendData(uint16_t opcode, int size, void *data);

std::vector<std::tuple<std::string, int, char>> ControlSocketGetProcesses();