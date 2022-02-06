#include "ProjectBuilder.h"

#include <cstdio>
#include <filesystem>
#include <thread>

#include "ConsoleApp.h"
#include "Content.h"
#include "Libdragon.h"
#include "generated/generated.h"
#include "static/static.h"

extern ConsoleApp console;
extern bool open_project(const char *path);

void create_project_thread(std::string project_folder) {
	// create folder if it doesn't exist
	const std::filesystem::path project_path(project_folder);
	if (!std::filesystem::exists(project_path)) {
		console.AddLog("Creating project folder...");
		std::filesystem::create_directories(project_path);
	}

	console.AddLog("Running 'libdragon init' at '%s'...", project_folder.c_str());
	console.AddLog("Check output on the console...");

	Libdragon::InitSync(project_folder);

	console.AddLog("Libdragon initialized.");

	console.AddLog("Adding/Removing starting files...");

	std::filesystem::path old_main_c_file(project_folder + "/src/main.c");
	if (std::filesystem::exists(old_main_c_file)) {
		std::filesystem::remove(old_main_c_file);
	}

	std::filesystem::path vscode_path(project_folder + "/.vscode");
	if (!std::filesystem::exists(vscode_path)) {
		std::filesystem::create_directories(vscode_path);
	}

	ProjectBuilder::GenerateStaticFiles(project_folder);

	console.AddLog("Adding libdragon-extensions...");

	char cmd[500];
	snprintf(cmd, 500,
			 "cd %s\ngit submodule add git@github.com:stefanmielke/libdragon-extensions.git "
			 "libs/libdragon-extensions",
			 project_folder.c_str());
	system(cmd);

	console.AddLog("Creating project settings file...");

	ProjectSettings default_project_settings;
	default_project_settings.project_directory = project_folder;
	default_project_settings.SaveToDisk();

	console.AddLog("Project settings file created.");

	console.AddLog("Project creation complete.");

	open_project(project_folder.c_str());
}

void ProjectBuilder::Create(std::string project_folder) {
	std::thread(create_project_thread, project_folder).detach();
}

void create_build_files(ProjectSettings &project_settings, Project &project,
						std::vector<std::unique_ptr<LibdragonImage>> &images,
						std::vector<std::unique_ptr<LibdragonSound>> &sounds) {
	std::filesystem::remove_all(project_settings.project_directory + "/build");

	std::string makefile_path(project_settings.project_directory + "/Makefile");
	generate_makefile_gen(makefile_path, project_settings.rom_name.c_str(),
						  project_settings.project_name.c_str(), !images.empty());

	std::string setup_path(project_settings.project_directory + "/src/setup.gen.c");
	generate_setup_gen_c(setup_path, project_settings);

	std::string change_scene_path(project_settings.project_directory +
								  "/src/scenes/change_scene.gen.c");
	generate_change_scene_gen_c(change_scene_path, project);

	generate_scene_gen_files(project_settings, project);

	Content::CreateSprites(project_settings, images);
	Content::CreateSounds(project_settings, sounds);
}

void ProjectBuilder::Build(ProjectSettings &project_settings, Project &project,
						   std::vector<std::unique_ptr<LibdragonImage>> &images,
						   std::vector<std::unique_ptr<LibdragonSound>> &sounds) {
	create_build_files(project_settings, project, images, sounds);

	Libdragon::Build(project_settings.project_directory);
}

void ProjectBuilder::Rebuild(ProjectSettings &project_settings, Project &project,
							 std::vector<std::unique_ptr<LibdragonImage>> &images,
							 std::vector<std::unique_ptr<LibdragonSound>> &sounds) {
	Libdragon::CleanSync(project_settings.project_directory);

	create_build_files(project_settings, project, images, sounds);

	Libdragon::Build(project_settings.project_directory);
}

void ProjectBuilder::GenerateStaticFiles(std::string project_folder) {
	std::filesystem::create_directories(project_folder + "/.vscode");
	std::filesystem::create_directories(project_folder + "/src/scenes");

	create_static_file(project_folder + "/.vscode/c_cpp_properties.json", vs_code_cpp_properties);

	create_static_file(project_folder + "/.clang-format", clang_format);
	create_static_file(project_folder + "/.gitignore", gitignore);

	create_static_file(project_folder + "/src/main.s.c", main_s_c);
	create_static_file(project_folder + "/src/game.s.h", game_s_h);
	create_static_file(project_folder + "/src/scenes/change_scene.s.h", change_scene_s_h);
}
