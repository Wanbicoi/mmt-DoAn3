#include <filesystem>
#include <string>
#include <vector>
#include <system_error>
namespace fs = std::filesystem;

std::vector<char> filesystem_get_drives() {
	std::vector<char> drives;
	for (char letter = 'A'; letter <= 'Z'; ++letter) {
		fs::path drive_path = fs::path(std::string(1, letter) + ":\\");
		if (fs::exists(drive_path)) {
			drives.push_back(letter);
		}
	}
	return drives;
}

std::string filesystem_get_default_location() {
	return std::string(getenv("USERPROFILE"));
}

std::vector<FileInfo> filesystem_list(std::string path) {
	if (path == "") { //Drives
		auto drive_letters = filesystem_get_drives();
		std::vector<FileInfo> drives;
		for (const auto &letter: drive_letters)
			drives.push_back({std::string(1, letter), ENTRY_DRIVE});
		return drives;
	} //else
	
	fs::path origin(path);
	std::vector<FileInfo> folders(1, {"..", ENTRY_PARENT});
	std::vector<FileInfo> files;
	for (const auto& entry: fs::directory_iterator(path)) {
		if (fs::is_directory(entry.path()))
			folders.push_back({entry.path().filename().string(), ENTRY_FOLDER});
		else 
			files.push_back({entry.path().filename().string(), ENTRY_FILE});
	}
	//Folders first, then files
	folders.insert(folders.end(), files.begin(), files.end());
	
	return folders;
}

bool is_folder(const fs::path &path) {
	return fs::exists(path) && fs::is_directory(path);
}

bool is_file(const fs::path &path) {
	//Clump symlink and stuff together as file. Further error will be processed by move/copy operation
	return fs::exists(path) && !fs::is_directory(path);

	//return fs::exists(filePath) && fs::is_regular_file(filePath);
}

bool filesystem_check_exist_file(const std::string &from, const std::string &to) {
	fs::path source_file(from);
	fs::path dest_file(to);

	if (is_file(dest_file)) {
		//std::cout << "Error: File with the same name already exists in the destination directory." << std::endl;
		return 1;
	}
	return 0;
}

//Whether the destination to already has sub-files of the same name as from's sub-files
bool filesystem_check_exist_dir(const std::string &from, const std::string &to) {
	fs::path source_dir(from);
	fs::path dest_dir(to);

	for (const auto& entry : fs::recursive_directory_iterator(source_dir)) {
		if (is_file(entry.path())) {
			fs::path entry_path = dest_dir / entry.path().lexically_relative(source_dir);
			if (is_file(entry_path)) {
				//std::cout << "Error: File with the same name already exists in the destination folder." << std::endl;
				return 1;
			}
		}
	}
	return 0;
}

bool filesystem_check_exist(const std::string &from, const std::string &to) {
	fs::path from_path(from);
	fs::path to_path(to);

	if (is_file(from_path) && is_file(to_path)) {
		return filesystem_check_exist_file(from, to);
	}
	else if (is_folder(from_path) && is_folder(to_path)) {
		return filesystem_check_exist_dir(from, to);
	}
	else {
		std::cout << "from and to must be same type of path" << std::endl;
	}

	return 1;
}

//parent should not end with slash
int filesystem_copy(const std::string &from, const std::string &to, bool overwrite) {
	auto copy_options = fs::copy_options::recursive;
	if (overwrite) copy_options |= fs::copy_options::overwrite_existing;
	else copy_options |= fs::copy_options::skip_existing;
	std::error_code error;

	fs::copy(from, to, copy_options, error);
	return error.value();
}

int filesystem_rename(const std::string &from, const std::string &to, bool overwrite) {
	std::error_code error;

	fs::path from_path(from);
	fs::path to_path(to);

	//fs::rename on folder requires destination folder to not exist or empty to perform simple relink.
	//Windows allow merging when doing it via Explorer. This part replicate it
	if (is_folder(from_path) && is_folder(to_path)) {
		for (const auto& entry : fs::recursive_directory_iterator(from_path)) {
			fs::path to_entry_path = to_path / entry.path().lexically_relative(from_path);
			if (is_file(entry.path())) {
				std::cout << "Entry " << to_entry_path << std::endl;
				if (is_file(to_entry_path)) {
					if (overwrite)
						fs::rename(entry.path(), to_entry_path, error);
				}
				else {
					fs::rename(entry.path(), to_entry_path, error);
				}
			}
			else if (is_folder(entry.path())) {
				fs::create_directories(to_entry_path, error);
			}
		}
	}
	else
		fs::rename(from, to, error);
	return error.value();
}

int filesystem_write(const std::string &path, bool overwrite, void *data, size_t size) {

}

int filesystem_delete(const std::string &path, int *deleted) {
	std::error_code error;
	*deleted = fs::remove_all(path, error);
	return error.value();
}