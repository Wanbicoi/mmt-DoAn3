#include <filesystem>
#include <string>
#include <vector>
namespace fs = std::filesystem;

std::vector<char> filesystem_get_drives() {
	std::vector<char> drives;
	for (char letter = 'A'; letter <= 'Z'; ++letter) {
		fs::path drivePath = fs::path(std::string(1, letter) + ":\\");
		if (fs::exists(drivePath)) {
			drives.push_back(letter);
		}
	}
}

std::string filesystem_get_default_location() {
	return std::string(getenv("USERPROFILE"));
}

std::vector<std::string> filesystem_list(std::string path) {
	std::vector<std::string> items;
	for (const auto& entry : fs::directory_iterator(path)) {
		items.push_back(entry.path().string());
	}
}

bool filesystem_check_exist_file(std::string from, std::string to) {

}

bool filesystem_check_exist_dir(std::string from, std::string to) {

}

bool filesystem_check_exist(std::string from, std::string to) {

}

bool filesystem_copy_skip_existing(std::string from, std::string to) {

}

bool filesystem_copy_overwrite_existing(std::string from, std::string to) {

}

bool filesystem_move_skip_existing(std::string from, std::string to) {

}

bool filesystem_move_overwrite_existing(std::string from, std::string to) {

}

bool filesystem_write_skip_existing(std::string path, void *data, size_t size) {

}

bool filesystem_write_overwrite_existing(std::string path, void *data, size_t size) {

}

bool filesystem_delete(std::string path) {
	
}