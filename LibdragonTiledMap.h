#pragma once

#include <string>

class LibdragonTiledMap {
   public:
	std::string name;
	std::string file_path;
	std::string dfs_folder;

	LibdragonTiledMap();

	void SaveToDisk(const std::string &project_directory);
	void LoadFromDisk(const std::string &filepath);
	void DeleteFromDisk(const std::string &project_directory) const;

	void DrawTooltip() const;
};
