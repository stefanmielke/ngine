#include "Content.h"

#include <filesystem>
#include <sstream>

#include "ConsoleApp.h"
#include "ThreadCommand.h"

void Content::CreateSprites(ProjectSettings &project_settings,
							std::vector<std::unique_ptr<LibdragonImage>> &images) {
	std::stringstream command;
	command << "cd " << project_settings.project_directory << std::endl;

	for (auto &image : images) {
		std::string dfs_output_path = "build/filesystem" + image->dfs_folder;
		std::filesystem::create_directories(project_settings.project_directory + "/" +
											dfs_output_path);

		command << "libdragon exec /n64_toolchain/bin/mksprite "
				<< (project_settings.display.bit_depth == DEPTH_16_BPP ? 16 : 32) << " "
				<< image->h_slices << " " << image->v_slices << " " << image->image_path << " "
				<< dfs_output_path + image->name + ".sprite" << std::endl;
	}

	console.AddLog("Building assets...\n%s", command.str().c_str());

	ThreadCommand::RunCommand(command.str());
}
void Content::CreateSounds(ProjectSettings &project_settings,
						   std::vector<std::unique_ptr<LibdragonSound>> &sounds) {
	std::stringstream command;
	command << "cd " << project_settings.project_directory << std::endl;

	for (auto &sound : sounds) {
		std::string dfs_output_path = "build/filesystem" + sound->dfs_folder;
		std::filesystem::create_directories(project_settings.project_directory + "/" +
											dfs_output_path);

		command << "libdragon exec /n64_toolchain/bin/audioconv64 " << sound->GetLibdragonGenFlags()
				<< " -o " << dfs_output_path + sound->name + sound->GetLibdragonExtension() << " "
				<< sound->sound_path << std::endl;
	}

	console.AddLog("Building assets...\n%s", command.str().c_str());

	ThreadCommand::RunCommand(command.str());
}
