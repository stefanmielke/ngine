#pragma once

#include <memory>
#include <string>
#include <SDL2/SDL_image.h>

enum LibdragonImageType {
	IMAGE_UNKNOWN,
	IMAGE_PNG,
	IMAGE_BMP,
	IMAGE_JPG,
	IMAGE_JPEG,
	IMAGE_PCX,
	IMAGE_TGA,
};

std::string get_libdragon_image_type_name(LibdragonImageType type);
std::string get_libdragon_image_type_extension(LibdragonImageType type);

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

	LibdragonImageType type;

	SDL_Texture *loaded_image;

	LibdragonImage();
	~LibdragonImage();

	void LoadImage(const std::string &project_directory, SDL_Renderer *renderer);

	void SaveToDisk(const std::string &project_directory);
	void LoadFromDisk(const std::string &filepath);
	void DeleteFromDisk(const std::string &project_directory) const;

	void DrawTooltip() const;

	bool operator<(const LibdragonImage &other) const {
		return (name < other.name);
	}
};
