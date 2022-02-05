#pragma once

#include <string>

class LibdragonImage {
   public:
	std::string name;
	std::string image_path;
	std::string dfs_folder;
	int h_slices;
	int v_slices;

	LibdragonImage();

	void SaveToDisk(std::string project_directory);
	void LoadFromDisk(std::string filepath);
};
