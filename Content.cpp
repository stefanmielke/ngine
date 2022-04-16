#include "Content.h"

#include <filesystem>
#include <sstream>

#include "App.h"
#include "ConsoleApp.h"
#include "ThreadCommand.h"

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
		command << engine_settings.GetLibdragonExeLocation() << " exec /n64_toolchain/bin/mksprite "
				<< (project_settings.display.bit_depth == DEPTH_16_BPP ? 16 : 32) << " "
				<< image->h_slices << " " << image->v_slices << " " << build_temp_image_path << " "
				<< dfs_output_path + image->name + ".sprite";

		console.AddLog("%s", command.str().c_str());
		ThreadCommand::QueueCommand(command.str());
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

		command << engine_settings.GetLibdragonExeLocation()
				<< " exec /n64_toolchain/bin/audioconv64 " << sound->GetLibdragonGenFlags()
				<< " -o " << dfs_output_path + sound->name + sound->GetLibdragonExtension() << " "
				<< sound->sound_path;

		console.AddLog("%s", command.str().c_str());
		ThreadCommand::QueueCommand(command.str());
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

			console.AddLog("%s", command.str().c_str());
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
		command << engine_settings.GetLibdragonExeLocation() << " exec /n64_toolchain/bin/mksprite "
				<< (project_settings.display.bit_depth == DEPTH_16_BPP ? 16 : 32) << " 16 8 "
				<< build_temp_image_path << " " << dfs_output_path + font->name + ".font";

		console.AddLog("%s", command.str().c_str());
		ThreadCommand::QueueCommand(command.str());
	}
}
