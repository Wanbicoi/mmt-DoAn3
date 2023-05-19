#include <string>
#include <vector>
#include "types.h"

class ScreenClient {
private:
	ScreenInfo screen_info = {1080, 720};
public:
	ScreenClient();

	void connect(const char *address);

	int getWidth();

	int getHeight();

	void getScreenData(void *data);

	int getMouseInfo(int *mouse_x, int *mouse_y);

	unsigned char* getMouseData(int *mouse_width, int *mouse_height);

	~ScreenClient();
};

class ControlClient {
private:
	void getData(void *data, int size);

	std::string getString();
public:
	ControlClient();

	void connect(const char *address);

	void sendControl(OperationCode opcode, int data = 0, void *raw_data = NULL);

	std::vector<ProcessInfo> getProcesses();

	~ControlClient();
};

void IoContextRun();