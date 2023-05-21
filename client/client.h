#include <string>
#include <vector>
#include <atomic>
#include <system_error>
#include "types.h"

class ScreenClient {
private:
	bool connected = 0;

	ScreenInfo screen_info = {1080, 720};

	unsigned char *screen_data = nullptr;
	std::atomic<bool> screen_changed = 0;

	unsigned char *mouse_data = nullptr;
	std::atomic<bool> mouse_changed = 0;
	std::atomic<int> mouse_x = 0;
	std::atomic<int> mouse_y = 0;
	std::atomic<int> mouse_width = 0;
	std::atomic<int> mouse_height = 0;
	std::atomic<int> mouse_center_x = 0;
	std::atomic<int> mouse_center_y = 0;

	void getData(void *data, int size);

	void handleRead(std::error_code error);
public:
	ScreenClient();

	void connect(const char *address);

	bool isConnected();

	void init();

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
	bool connected = 0;

	void getData(void *data, int size);

	void getString(std::string *str);

	void sendData(void *data, int size);

	void sendString(std::string str);

	void sendControl(OperationCode opcode);
public:
	ControlClient();

	void connect(const char *address);

	bool isConnected();

	void suspendProcess(int pid);

	void resumeProcess(int pid);

	void terminateProcess(int pid);

	std::vector<ProcessInfo> getProcesses();

	std::string getDefaultLocation();

	std::vector<FileInfo> listDir(std::string path);

	bool checkExist(std::string from, std::string to);

	void requestCopy(std::string from, std::string to, bool overwrite);

	void requestMove(std::string from, std::string to, bool overwrite);

	void requestDelete(std::string path);

	void sendDisconnect();

	~ControlClient();
};

void IoContextRun();

void IoContextStop();