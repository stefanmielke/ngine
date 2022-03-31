#include "Content.h"

#include <filesystem>
#include <sstream>

#include "ConsoleApp.h"
#include "ThreadCommand.h"

void Content::CreateSprites(const EngineSettings &engine_settings,
							const ProjectSettings &project_settings,
							const std::vector<std::unique_ptr<LibdragonImage>> &images) {
	if (images.empty())
		return;

	std::stringstream command;
	command << "cd " << project_settings.project_directory << separator;

	for (auto &image : images) {
		std::string dfs_output_path = "build/filesystem" + image->dfs_folder;
		std::filesystem::create_directories(project_settings.project_directory + "/" +
											dfs_output_path);

		command << engine_settings.GetLibdragonExeLocation() << " exec /n64_toolchain/bin/mksprite "
				<< (project_settings.display.bit_depth == DEPTH_16_BPP ? 16 : 32) << " "
				<< image->h_slices << " " << image->v_slices << " \"" << image->image_path << "\" \""
				<< dfs_output_path + image->name + ".sprite\"" << std::endl;
	}

	console.AddLog("Building assets...\n%s", command.str().c_str());

	ThreadCommand::RunCommand(command.str());
}
void Content::CreateSounds(const EngineSettings &engine_settings,
						   const ProjectSettings &project_settings,
						   const std::vector<std::unique_ptr<LibdragonSound>> &sounds) {
	if (sounds.empty())
		return;

	std::stringstream command;
	command << "cd " << project_settings.project_directory << separator;

	for (auto &sound : sounds) {
		std::string dfs_output_path = "build/filesystem" + sound->dfs_folder;
		std::filesystem::create_directories(project_settings.project_directory + "/" +
											dfs_output_path);

		command << engine_settings.GetLibdragonExeLocation()
				<< " exec /n64_toolchain/bin/audioconv64 " << sound->GetLibdragonGenFlags()
				<< " -o " << dfs_output_path + sound->name + sound->GetLibdragonExtension() << " "
				<< sound->sound_path << std::endl;
	}

	console.AddLog("Building assets...\n%s", command.str().c_str());

	ThreadCommand::RunCommand(command.str());
}

void Content::CreateGeneralFiles(const EngineSettings &engine_settings,
								 const ProjectSettings &project_settings,
								 const std::vector<std::unique_ptr<LibdragonFile>> &files) {
	if (files.empty())
		return;

	std::stringstream command;
	command << "cd " << project_settings.project_directory << separator;

	for (auto &file : files) {
		if (file->copy_to_filesystem) {
			std::string dfs_output_path = "build/filesystem" + file->dfs_folder;
			std::filesystem::create_directories(project_settings.project_directory + "/" +
												dfs_output_path);

			command << "cp " << file->file_path << " "
					<< dfs_output_path + file->name + file->file_type << std::endl;
		}
	}

	console.AddLog("Building general files...\n%s", command.str().c_str());

	ThreadCommand::RunCommand(command.str());
}
