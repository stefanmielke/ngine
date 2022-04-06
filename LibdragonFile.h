#pragma once

#include <memory>
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

	[[nodiscard]] std::string GetFilename() const;
	[[nodiscard]] std::string GetTooltip() const;

	bool operator<(const LibdragonFile &other) const {
		return (name < other.name);
	}
};

bool libdragon_file_comparison(const std::unique_ptr<LibdragonFile> &f1,
							   const std::unique_ptr<LibdragonFile> &f2);
