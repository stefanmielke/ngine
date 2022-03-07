#pragma once

#include <string>

class LibdragonFile {
   public:
	std::string name;
	std::string file_type;
	std::string file_path;
	std::string dfs_folder;
	bool copy_to_filesystem;

	LibdragonFile();

	void SaveToDisk(const std::string &project_directory);
	void LoadFromDisk(const std::string &filepath);
	void DeleteFromDisk(const std::string &project_directory) const;

	[[nodiscard]] std::string GetTooltip() const;
};
