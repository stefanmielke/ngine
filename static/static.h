#pragma once

#include <fstream>

extern const char *clang_format;
extern const char *gitignore;
extern const char *main_s_cpp;
extern const char *vs_code_cpp_properties;

void create_static_file(std::string filepath, const char *file_content) {
	std::ofstream filestream(filepath);
	filestream << file_content << std::endl;
	filestream.close();
}