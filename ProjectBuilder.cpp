#include <filesystem>
#include <fstream>

#include "ProjectBuilder.h"

#include "static/static.h"

void ProjectBuilder::Build(ProjectSettings project_settings) {
	std::filesystem::path old_main_c_file(project_settings.project_directory + "/src/main.c");
	if (std::filesystem::exists(old_main_c_file)) {
		std::filesystem::remove(old_main_c_file);
	}

	std::ofstream main_s_cpp_file(project_settings.project_directory + "/src/main.c");
	main_s_cpp_file << main_s_cpp << std::endl;
	main_s_cpp_file.close();

	char command[500];
	snprintf(command, 500, "cd %s\nlibdragon make", project_settings.project_directory.c_str());
	system(command);
}
