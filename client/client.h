#include <string>
#include <vector>
#include <tuple>

int ScreenSocketGetWidth();

int ScreenSocketGetHeight();

void ScreenSocketConnect(const char *address);

void ScreenSocketGetData(void *data, size_t size);

void ScreenSocketClose();

void ControlSocketConnect(const char *address);

void ControlSocketClose();

void ControlSocketSendData(uint16_t opcode, int size, void *data);

std::vector<std::tuple<std::string, int, char>> ControlSocketGetProcesses();