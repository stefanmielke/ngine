#include "ProjectBuilder.h"

#include <cstdio>
#include <filesystem>
#include <fstream>

#include "Libdragon.h"
#include "generated/generated.h"
#include "static/static.h"

void ProjectBuilder::Build(ProjectSettings project_settings) {
	std::filesystem::path old_main_c_file(project_settings.project_directory + "/src/main.c");
	if (std::filesystem::exists(old_main_c_file)) {
		std::filesystem::remove(old_main_c_file);
	}

	std::ofstream main_s_cpp_file(project_settings.project_directory + "/src/main.s.c");
	main_s_cpp_file << main_s_cpp << std::endl;
	main_s_cpp_file.close();

	std::string makefile_path(project_settings.project_directory + "/Makefile");
	FILE *makefile = fopen(makefile_path.c_str(), "w");
	generate_makefile_gen_cpp(makefile, project_settings.rom_name.c_str(), project_settings.project_name.c_str());
	fclose(makefile);

	Libdragon::Build(project_settings.project_directory);
}
