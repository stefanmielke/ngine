#pragma once

#include <cstdio>
#include <string>

#include "../settings/Project.h"
#include "../settings/ProjectSettings.h"

extern const char *script_blank_gen_h;
extern const char *script_blank_gen_c;

void generate_makefile_gen(std::string &makefile_path, const char *rom_filename, const char *rom_title);
void generate_setup_gen_c(std::string &setup_path, ProjectSettings &settings);
void generate_change_scene_gen_c(std::string &filepath, Project &project);
void generate_scene_gen_files(std::string &project_folder, Project &project);