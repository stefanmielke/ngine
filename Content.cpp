#include "Content.h"

#include <filesystem>
#include <fstream>
#include <sstream>

#include <LDtkLoader/Project.hpp>

#include "App.h"
#include "ConsoleApp.h"
#include "Libdragon.h"
#include "ThreadCommand.h"
#include "pugixml/pugixml.hpp"

extern App *g_app;

void Content::CreateSprites(const EngineSettings &engine_settings,
							const ProjectSettings &project_settings,
							const std::vector<std::unique_ptr<LibdragonImage>> &images) {
	if (images.empty())
		return;

	console.AddLog("Building sprite assets...");

	for (auto &image : images) {
		std::stringstream command;
		std::string dfs_output_path = "build/filesystem" + image->dfs_folder;
		std::filesystem::create_directories(project_settings.project_directory + "/" +
											dfs_output_path);

		// copy to temp folder (build/temp/sprites) as png
		std::string temp_directory = project_settings.project_directory + "/build/temp/sprites/";
		std::string temp_filepath = temp_directory + image->name + ".png";
		std::filesystem::create_directories(temp_directory);

		std::string image_full_path = project_settings.project_directory + "/" + image->image_path;
		SDL_Surface *image_surface = IMG_Load(image_full_path.c_str());
		IMG_SavePNG(image_surface, temp_filepath.c_str());
		SDL_FreeSurface(image_surface);

		std::string build_temp_image_path = "build/temp/sprites/" + image->name + ".png";
		command << "/n64_toolchain/bin/mksprite "
				<< (project_settings.display.bit_depth == DEPTH_16_BPP ? 16 : 32) << " "
				<< image->h_slices << " " << image->v_slices << " " << build_temp_image_path << " "
				<< dfs_output_path + image->name + ".sprite";

		Libdragon::Exec(g_app, command.str());
	}
}

void Content::CreateSounds(const EngineSettings &engine_settings,
						   const ProjectSettings &project_settings,
						   const std::vector<std::unique_ptr<LibdragonSound>> &sounds) {
	if (sounds.empty())
		return;

	console.AddLog("Building sound assets...");

	for (auto &sound : sounds) {
		std::stringstream command;
		std::string dfs_output_path = "build/filesystem" + sound->dfs_folder;
		std::filesystem::create_directories(project_settings.project_directory + "/" +
											dfs_output_path);

		command << "/n64_toolchain/bin/audioconv64 " << sound->GetLibdragonGenFlags() << " -o "
				<< dfs_output_path + sound->name + sound->GetLibdragonExtension() << " "
				<< sound->sound_path;

		Libdragon::Exec(g_app, command.str());
	}
}

void Content::CreateGeneralFiles(const EngineSettings &engine_settings,
								 const ProjectSettings &project_settings,
								 const std::vector<std::unique_ptr<LibdragonFile>> &files) {
	if (files.empty())
		return;

	console.AddLog("Building general assets...");

	for (auto &file : files) {
		if (file->copy_to_filesystem) {
			std::stringstream command;
			std::string dfs_output_path = "build/filesystem" + file->dfs_folder;
			std::filesystem::create_directories(project_settings.project_directory + "/" +
												dfs_output_path);

			command << "cp \"" << file->file_path << "\" \""
					<< dfs_output_path + file->GetFilename() << "\"";

			ThreadCommand::QueueCommand(command.str());
		}
	}
}

void Content::CreateFonts(const EngineSettings &engine_settings,
						  const ProjectSettings &project_settings,
						  const std::vector<std::unique_ptr<LibdragonFont>> &fonts) {
	if (fonts.empty())
		return;

	console.AddLog("Building font assets...");

	for (auto &font : fonts) {
		std::stringstream command;
		std::string dfs_output_path = "build/filesystem" + font->dfs_folder;
		std::filesystem::create_directories(project_settings.project_directory + "/" +
											dfs_output_path);

		// copy to temp folder (build/temp/sprites) as png
		std::string temp_directory = project_settings.project_directory + "/build/temp/fonts/";
		std::string temp_filepath = temp_directory + font->name + ".png";
		std::filesystem::create_directories(temp_directory);

		std::string image_full_path = project_settings.project_directory + "/" + font->font_path;
		SDL_Surface *image_surface = LibdragonFont::LoadSurfaceFromFont(
			image_full_path.c_str(), font->font_size, g_app->renderer);
		IMG_SavePNG(image_surface, temp_filepath.c_str());
		SDL_FreeSurface(image_surface);

		std::string build_temp_image_path = "build/temp/fonts/" + font->name + ".png";
		command << "/n64_toolchain/bin/mksprite "
				<< (project_settings.display.bit_depth == DEPTH_16_BPP ? 16 : 32) << " 16 8 "
				<< build_temp_image_path << " " << dfs_output_path + font->name + ".font";

		Libdragon::Exec(g_app, command.str());
	}
}

void Content::CreateTiledMaps(const EngineSettings &engine_settings,
							  const ProjectSettings &project_settings,
							  const std::vector<std::unique_ptr<LibdragonTiledMap>> &maps) {
	if (maps.empty())
		return;

	console.AddLog("Building tile maps assets...");

	for (auto &map : maps) {
		std::string dfs_output_path = "build/filesystem" + map->dfs_folder;
		std::filesystem::create_directories(project_settings.project_directory + "/" +
											dfs_output_path);

		std::string file_path = project_settings.project_directory + "/" + map->file_path;

		pugi::xml_document tmx_file;

		auto result = tmx_file.load_file(file_path.c_str());
		if (result.status != pugi::status_ok) {
			console.AddLog("Error loading tiled map: %s", result.description());
			continue;
		}

		auto xml_layers = tmx_file.child("map").children("layer");
		for (auto &xml_layer : xml_layers) {
			std::string output_file_path = dfs_output_path + map->name + "_" +
										   xml_layer.attribute("name").value() + ".map";

			int width = xml_layer.attribute("width").as_int();

			std::stringstream layer_map;
			std::string tile_string;
			int cur_width = 0;

			std::string csv_map = xml_layer.child_value("data");
			std::istringstream csv_stream(csv_map);
			while (std::getline(csv_stream, tile_string, ',')) {
				int tile = std::stoi(tile_string);
				--tile;

				layer_map << tile;

				++cur_width;
				if (cur_width >= width) {
					cur_width = 0;
					layer_map << "\n";
				} else {
					layer_map << ",";
				}
			}

			std::ofstream layer_file(output_file_path);
			layer_file << layer_map.str();
			layer_file.close();

			console.AddLog("Created map file at %s", output_file_path.c_str());
		}
	}
}

void Content::CreateLDtkMaps(const EngineSettings &engine_settings,
							 const ProjectSettings &project_settings,
							 const std::vector<std::unique_ptr<LibdragonLDtkMap>> &maps) {
	if (maps.empty())
		return;

	console.AddLog("Building ldtk maps assets...");

	for (auto &map : maps) {
		std::string dfs_output_path = "build/filesystem" + map->dfs_folder;
		std::filesystem::create_directories(project_settings.project_directory + "/" +
											dfs_output_path);

		std::string file_path = project_settings.project_directory + "/" + map->file_path;

		ldtk::Project project;

		try {
			project.loadFromFile(file_path);
		} catch (std::exception &ex) {
			console.AddLog("Error loading ldtk map: %s", ex.what());
			continue;
		}

		std::vector<LibdragonMapLayer> layers;

		for (auto &world : project.allWorlds()) {
			for (const auto &level : world.allLevels()) {
				std::string level_name(level.name);

				std::string level_output_dir = project_settings.project_directory + "/" +
											   dfs_output_path + level_name;
				std::filesystem::create_directories(level_output_dir);

				for (const auto &layer : level.allLayers()) {
					if (layer.allTiles().empty())
						continue;

					std::string layer_output_dir = level_output_dir + "/" + layer.getName() +
												   ".map";

					{
						const size_t map_size = layer.getGridSize().x * layer.getGridSize().y;
						std::vector<int> map_output;
						map_output.reserve(map_size);
						for (size_t i = 0; i < map_size; ++i) {
							map_output.push_back(-1);
						}

						for (const auto &tile : layer.allTiles()) {
							map_output[tile.coordId] = tile.tileId;
						}

						std::ofstream file(layer_output_dir);

						int coord_id = 0;
						for (auto &tile : map_output) {
							file << tile;

							if (coord_id % layer.getGridSize().x == layer.getGridSize().x - 1)
								file << "\n";
							else
								file << ",";

							++coord_id;
						}

						// replace last ',' to a new line
						file.seekp(file.tellp() - std::streampos(1));
						file << "\n";

						file.close();

						console.AddLog("Created map file at %s", layer_output_dir.c_str());
					}
				}
			}
		}
	}
}