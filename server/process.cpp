#include <windows.h>
#include <tlhelp32.h>
#include <vector>
#include <utility>
#include <string>
#include <sstream>
#include <iostream>

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

std::string list_running_processes() {
	std::stringstream ss;
	for (auto process: get_current_processes()) {
		ss << process.first << " (PID: " << process.second << ")\n";
	}
	return ss.str();
}

int main() {
	std::cout << list_running_processes();
	return 0;
}