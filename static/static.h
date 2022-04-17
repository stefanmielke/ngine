#pragma once

#include <fstream>

extern const char *change_scene_s_h;
extern const char *clang_format;
extern const char *gitignore;
extern const char *main_s_c;
extern const char *makefile_custom_mk;
extern const char *vs_code_cpp_properties;

inline void create_static_file(const std::string& filepath, const char *file_content) {
	std::ofstream filestream(filepath);
	filestream << file_content << std::endl;
	filestream.close();
}