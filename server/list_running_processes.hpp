#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>
#include <string>
#include <sstream>

// To ensure correct resolution of symbols, add Psapi.lib to TARGETLIBS
// and compile with -DPSAPI_VERSION=1
std::string PrintProcessNameAndID(DWORD processID)
{
    TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

    // Get a handle to the process.

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
                                      PROCESS_VM_READ,
                                  FALSE, processID);

    // Get the process name.

    if (NULL != hProcess)
    {
        HMODULE hMod;
        DWORD cbNeeded;

        if (EnumProcessModules(hProcess, &hMod, sizeof(hMod),
                               &cbNeeded))
        {
            GetModuleBaseName(hProcess, hMod, szProcessName,
                              sizeof(szProcessName) / sizeof(TCHAR));
        }
    }

    // Print the process name and identifier.
    std::stringstream ss;
    ss << szProcessName << " (PID: " << processID << ")";

    // Release the handle to the process.

    CloseHandle(hProcess);

    return ss.str();
}

std::string list_running_processes()
{
    // Get the list of process identifiers.

    DWORD aProcesses[1024], cbNeeded, cProcesses;
    unsigned int i;

    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
    {
        return "Somethign went wrong with enumeration of process identifiers";
    }

    // Calculate how many process identifiers were returned.

    cProcesses = cbNeeded / sizeof(DWORD);

    // Print the name and process identifier for each process.
    std::stringstream ss;
    for (i = 0; i < cProcesses; i++)
    {
        if (aProcesses[i] != 0)
        {
            ss << PrintProcessNameAndID(aProcesses[i]) << std::endl;
        }
    }

    return ss.str();
}