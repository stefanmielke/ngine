#pragma once

#include <string>
#include <vector>

struct LibdragonMapLayer {
	std::string name;
};

class LibdragonTiledMap {
   public:
	std::string name;
	std::string file_path;
	std::string dfs_folder;

	std::vector<LibdragonMapLayer> layers;

	LibdragonTiledMap();

	void SaveToDisk(const std::string &project_directory);
	void LoadFromDisk(const std::string &filepath);
	void DeleteFromDisk(const std::string &project_directory) const;

	void LoadLayers();
	static std::vector<LibdragonMapLayer> LoadLayers(const std::string &file_path);

	void DrawTooltip() const;
};
