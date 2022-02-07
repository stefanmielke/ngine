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

	void LoadImage(const std::string &project_directory, SDL_Renderer *renderer);

	void SaveToDisk(const std::string &project_directory);
	void LoadFromDisk(const std::string &filepath);
	void DeleteFromDisk(const std::string &project_directory) const;
};
