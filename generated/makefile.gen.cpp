#include "generated.h"

const char *makefile_gen_cpp =
	"V=1\n"
	"SOURCE_DIR=src\n"
	"BUILD_DIR=build\n"
	"include $(N64_INST)/include/n64.mk\n"
	"\n"
	"src=main.s.c\n"
	"\n"
	"all: %s.z64\n"
	"\n"
	"%s.z64: N64_ROM_TITLE=\"%s\"\n"
	"$(BUILD_DIR)/%s.elf: $(src:%%.c=$(BUILD_DIR)/%%.o)\n"
	"\n"
	"clean:\n"
	"\trm -f $(BUILD_DIR)/* %s.z64\n"
	"\n"
	"-include $(wildcard $(BUILD_DIR)/*.d)\n"
	"\n"
	".PHONY: all clean";

void generate_makefile_gen_cpp(FILE *filestream, const char *rom_filename, const char *rom_title) {
	fprintf(filestream, makefile_gen_cpp, rom_filename, rom_filename, rom_title, rom_filename,
			rom_filename);
}