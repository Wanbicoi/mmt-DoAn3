#include <string>
#include <vector>
#include <atomic>
#include <system_error>
#include "types.h"

class ScreenClient {
private:
	ScreenInfo screen_info = {1080, 720};

	unsigned char *screen_data = nullptr;
	std::atomic<bool> screen_changed = 0;

	unsigned char *mouse_data = nullptr;
	std::atomic<bool> mouse_changed = 0;
	std::atomic<int> mouse_x = 0;
	std::atomic<int> mouse_y = 0;
	std::atomic<int> mouse_width = 0;
	std::atomic<int> mouse_height = 0;


	void handleRead(std::error_code error);
public:
	ScreenClient();

	void connect(const char *address);

	int getWidth();

	int getHeight();

	bool isFrameChanged();

	bool isMouseImgChanged();

	unsigned char* getScreenData();

	void getMouseInfo(int *x, int *y);

	unsigned char* getMouseData(int *width, int *height);

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