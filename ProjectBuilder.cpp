#include "ProjectBuilder.h"

#include <cstdio>
#include <filesystem>
#include <thread>

#include "App.h"
#include "ConsoleApp.h"
#include "Content.h"
#include "Libdragon.h"
#include "ThreadCommand.h"
#include "generated/generated.h"
#include "static/static.h"

void create_project_thread(App *app, std::string project_folder) {
	app->project.project_settings.project_directory = project_folder;

	// create folder if it doesn't exist
	const std::filesystem::path project_path(project_folder);
	if (!std::filesystem::exists(project_path)) {
		console.AddLog("Creating project folder...");
		std::filesystem::create_directories(project_path);
	}

	console.AddLog("Running 'libdragon init' at '%s'...", project_folder.c_str());

	if (!Libdragon::InitSync(project_folder, app->engine_settings.GetLibdragonExeLocation())) {
		console.AddLog("[error] Failed to initialize libdragon.");
		return;
	}

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

	const char *lib_ext_submodule_cmd =
		"git submodule add https://github.com/stefanmielke/libdragon-extensions.git "
		"libs/libdragon-extensions";
	if (ThreadCommand::RunCommand(lib_ext_submodule_cmd) != EXIT_SUCCESS) {
		console.AddLog(
			"[error] Error adding libdragon-extensions. Please use the command below to add it "
			"manually.\n%s",
			lib_ext_submodule_cmd);
	}

	console.AddLog("Creating project settings file...");

	ProjectSettings default_project_settings;
	default_project_settings.project_directory = project_folder;
	default_project_settings.SaveToDisk();

	console.AddLog("Project settings file created.");

	console.AddLog("Project creation complete.");

	app->OpenProject(project_folder);
}

void ProjectBuilder::Create(App *app, std::string project_folder) {
	std::thread(create_project_thread, app, project_folder).detach();
}

void create_build_files(App *app) {
	std::filesystem::remove_all(app->project.project_settings.project_directory + "/build");

	generate_game_gen_h(app->project);

	generate_makefile_gen(app->project);

	std::string setup_path(app->project.project_settings.project_directory + "/src/setup.gen.c");
	generate_setup_gen_c(setup_path, app->project.project_settings);

	std::string change_scene_path(app->project.project_settings.project_directory +
								  "/src/scenes/change_scene.gen.c");
	generate_change_scene_gen_c(change_scene_path, app->project);

	generate_scene_gen_files(app->project);

	Content::CreateSprites(app->engine_settings, app->project.project_settings,
						   app->project.images);
	Content::CreateSounds(app->engine_settings, app->project.project_settings, app->project.sounds);
	Content::CreateGeneralFiles(app->engine_settings, app->project.project_settings,
								app->project.general_files);

	std::string path_to_content_script(app->project.project_settings.project_directory +
									   "/.ngine/pipeline/content_pipeline_end.term");
	if (std::filesystem::exists(path_to_content_script)) {
		console.AddLog("-- Custom content pipeline script starts --");

		std::ifstream content_end_script(path_to_content_script);

		std::string line;
		while (std::getline(content_end_script, line)) {
			char command[500];
			snprintf(command, 500, "%s", line.c_str());
			ThreadCommand::QueueCommand(line);

			console.AddLog("%s", command);
		}

		content_end_script.close();

		console.AddLog("-- Custom content pipeline script ends --");
	}
}

void ProjectBuilder::Build(App *app) {
	create_build_files(app);

	Libdragon::Build(app->project.project_settings.project_directory,
					 app->engine_settings.GetLibdragonExeLocation());
}

void ProjectBuilder::Rebuild(App *app) {
	Libdragon::CleanSync(app->project.project_settings.project_directory,
						 app->engine_settings.GetLibdragonExeLocation());

	create_build_files(app);

	Libdragon::Build(app->project.project_settings.project_directory,
					 app->engine_settings.GetLibdragonExeLocation());
}

void ProjectBuilder::GenerateStaticFiles(const std::string &project_folder) {
	std::filesystem::create_directories(project_folder + "/.vscode");
	std::filesystem::create_directories(project_folder + "/src/scenes");

	create_static_file(project_folder + "/.vscode/c_cpp_properties.json", vs_code_cpp_properties);

	create_static_file(project_folder + "/.clang-format", clang_format);
	create_static_file(project_folder + "/.gitignore", gitignore);

	create_static_file(project_folder + "/src/main.s.c", main_s_c);
	create_static_file(project_folder + "/src/scenes/change_scene.s.h", change_scene_s_h);
}
