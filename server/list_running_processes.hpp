#include <windows.h>
#include <tlhelp32.h>
#include <vector>
#include <utility>
#include <string>
#include <sstream>

// To ensure correct resolution of symbols, add Psapi.lib to TARGETLIBS
// and compile with -DPSAPI_VERSION=1
std::vector<std::pair<std::string, DWORD>> get_current_processes() {
	HANDLE hSnapshot;
	PROCESSENTRY32 pe;
	BOOL hResult;
	std::vector<std::pair<std::string, DWORD>> result;

	// snapshot of all processes in the system
	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE == hSnapshot) return {};

	// initializing size: needed for using Process32First
	pe.dwSize = sizeof(PROCESSENTRY32);

	// info about first process encountered in a system snapshot
	hResult = Process32First(hSnapshot, &pe);

	// retrieve information about the processes
	// and exit if unsuccessful
	while (hResult) {
		result.push_back(std::pair<std::string, DWORD>(std::string(pe.szExeFile), pe.th32ProcessID));
		hResult = Process32Next(hSnapshot, &pe);
	}

	// closes an open handle (CreateToolhelp32Snapshot)
	CloseHandle(hSnapshot);
	return result;
}


struct handle_data {
    unsigned long process_id;
    HWND window_handle;
};

BOOL is_main_window(HWND handle) {   
    return GetWindow(handle, GW_OWNER) == (HWND)0 && IsWindowVisible(handle);
}

BOOL CALLBACK enum_windows_callback(HWND handle, LPARAM lParam) {
    handle_data& data = *(handle_data*)lParam;
    unsigned long process_id = 0;
    GetWindowThreadProcessId(handle, &process_id);
    if (data.process_id != process_id || !is_main_window(handle))
        return TRUE;
    data.window_handle = handle;
    return FALSE;   
}

HWND find_main_window(unsigned long process_id) {
    handle_data data;
    data.process_id = process_id;
    data.window_handle = 0;
    EnumWindows(enum_windows_callback, (LPARAM)&data);
    return data.window_handle;
}
// To ensure correct resolution of symbols, add Psapi.lib to TARGETLIBS
// and compile with -DPSAPI_VERSION=1
std::vector<std::pair<std::string, DWORD>> get_current_applications() {
	HANDLE hSnapshot;
	PROCESSENTRY32 pe;
	BOOL hResult;
	std::vector<std::pair<std::string, DWORD>> result;

	// snapshot of all processes in the system
	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE == hSnapshot) return {};

	// initializing size: needed for using Process32First
	pe.dwSize = sizeof(PROCESSENTRY32);

	// info about first process encountered in a system snapshot
	hResult = Process32First(hSnapshot, &pe);

	// retrieve information about the processes
	// and exit if unsuccessful
	while (hResult) {
		if (IsWindowVisible(find_main_window(pe.th32ProcessID))) {
			result.push_back(std::pair<std::string, DWORD>(std::string(pe.szExeFile), pe.th32ProcessID));
		}
		hResult = Process32Next(hSnapshot, &pe);
	}

	// closes an open handle (CreateToolhelp32Snapshot)
	CloseHandle(hSnapshot);
	return result;
}