#pragma once

#include <cstdio>
#include <string>

#include "../settings/ProjectSettings.h"

void generate_makefile_gen_cpp(std::string makefile_path, const char *rom_filename, const char *rom_title);
void generate_setup_gen_c(std::string setup_path, ProjectSettings settings);