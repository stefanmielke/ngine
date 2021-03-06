#pragma once

#include <cstdio>
#include <string>

#include "../settings/Project.h"
#include "../settings/ProjectSettings.h"

extern const char *script_blank_gen_h;
extern const char *script_blank_gen_c;

void generate_makefile_gen(const Project &project);
void generate_setup_gen_c(std::string &setup_path, ProjectSettings &settings);
void generate_change_scene_gen_c(std::string &filepath, const Project &project);
void generate_scene_gen_files(const Project &project);
void generate_game_gen_h(const Project &project);