#pragma once

#include <string>
#include <SDL2/SDL_image.h>

class LibdragonImage {
   public:
	std::string name;
	std::string image_path;
	std::string dfs_folder;
	int h_slices;
	int v_slices;

	int width;
	int height;
	int display_width;
	int display_height;

	SDL_Texture *loaded_image;

	LibdragonImage();
	~LibdragonImage();

	void SaveToDisk(std::string &project_directory);
	void LoadFromDisk(std::string &filepath);
	void DeleteFromDisk(std::string &project_directory);
};
