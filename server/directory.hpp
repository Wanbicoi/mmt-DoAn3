#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <cstdio>
#include <filesystem>
#include <sstream>
#include <fstream>
namespace fs = std::filesystem;

class directory
{
private:
    std::string _current_directory;

public:
    directory();

    std::string list_files_in_current_directory();
    bool change_directory_to(const std::string name);
    bool goto_parent_directory();

    bool add_file(std::string file_path);
    bool delete_file(std::string file_name);
};

directory::directory()
{
    _current_directory = getenv("USERPROFILE");
}

inline std::string directory::list_files_in_current_directory()
{
    std::stringstream ss;
    for (const auto &entry : fs::directory_iterator(_current_directory))
        ss << entry.path() << std::endl;
    return ss.str();
}

inline bool directory::change_directory_to(const std::string name)
{
    _current_directory += name;
    return true;
}

inline bool directory::goto_parent_directory()
{
    std::size_t found = _current_directory.find_last_of("/\\", _current_directory.size() - 1);
    _current_directory = _current_directory.substr(0, found);
}

inline bool directory::add_file(std::string file_source, std::string file_name, char* data)
{
    
}

inline bool directory::delete_file(std::string file_name)
{
    return remove(file_name.c_str());
}
