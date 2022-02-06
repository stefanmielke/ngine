#pragma once

#include <string>

class LibdragonSound {
   public:
	std::string name;
	std::string sound_path;
	std::string dfs_folder;

	void SaveToDisk(std::string &project_directory);
	void LoadFromDisk(std::string &filepath);
	void DeleteFromDisk(std::string &project_directory) const;
};
