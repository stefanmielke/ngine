#include "ProjectBuilder.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <thread>

#include "ConsoleApp.h"
#include "Libdragon.h"
#include "generated/generated.h"
#include "static/static.h"

extern ConsoleApp console;

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
	std::ofstream c_cpp_properties_file(project_folder + "/.vscode/c_cpp_properties.json");
	c_cpp_properties_file << vs_code_cpp_properties << std::endl;
	c_cpp_properties_file.close();

	std::ofstream clang_format_file(project_folder + "/.clang-format");
	clang_format_file << clang_format << std::endl;
	clang_format_file.close();

	std::ofstream gitignore_file(project_folder + "/.gitignore");
	gitignore_file << gitignore << std::endl;
	gitignore_file.close();

	console.AddLog("Adding git submodules...");

	console.AddLog("Adding libdragon-extensions...");

	char cmd[500];
	snprintf(cmd, 500, "cd %s\ngit submodule add git@github.com:stefanmielke/libdragon-extensions.git libs/libdragon-extensions", project_folder.c_str());
	system(cmd);

	console.AddLog("Creating project settings file...");
	std::string project_settings_filename(project_folder + "/ngine.project.json");

	ProjectSettings default_project_settings;
	default_project_settings.SaveToFile(project_settings_filename);

	console.AddLog("Project settings file created.");

	console.AddLog("Project creation complete.");
}

void ProjectBuilder::Create(std::string project_folder) {
	std::thread(create_project_thread, project_folder).detach();
}

void create_build_files(ProjectSettings &project_settings) {
	std::ofstream main_s_cpp_file(project_settings.project_directory + "/src/main.s.c");
	main_s_cpp_file << main_s_cpp << std::endl;
	main_s_cpp_file.close();

	std::string makefile_path(project_settings.project_directory + "/Makefile");
	FILE *makefile = fopen(makefile_path.c_str(), "w");
	generate_makefile_gen_cpp(makefile, project_settings.rom_name.c_str(),
							  project_settings.project_name.c_str());
	fclose(makefile);
}

void ProjectBuilder::Build(ProjectSettings project_settings) {
	create_build_files(project_settings);

	Libdragon::Build(project_settings.project_directory);
}

void ProjectBuilder::Rebuild(ProjectSettings project_settings) {
	create_build_files(project_settings);

	Libdragon::Rebuild(project_settings.project_directory);
}
