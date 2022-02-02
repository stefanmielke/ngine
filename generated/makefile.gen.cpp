#include "generated.h"

const char *makefile_gen_cpp = R"(V=1
SOURCE_DIR=src
BUILD_DIR=build
include $(N64_INST)/include/n64.mk

N64_CFLAGS += -Ilibs/libdragon-extensions/include

C_ROOT_FILES := $(wildcard src/*.c)
C_LIB_EXTENSIONS_FILES := $(wildcard libs/libdragon-extensions/src/*.c)

SRC = $(C_ROOT_FILES) $(C_LIB_EXTENSIONS_FILES)
OBJS = $(SRC:%%.c=%%.o)
DEPS = $(SRC:%%.c=%%.d)

all: %s.z64

%s.z64: N64_ROM_TITLE="%s"
$(BUILD_DIR)/%s.elf: $(OBJS)

clean:
	find . -name '*.v64' -delete
	find . -name '*.z64' -delete
	find . -name '*.elf' -delete
	find . -name '*.o' -delete
	find . -name '*.d' -delete
	find . -name '*.bin' -delete
	find . -name '*.plan_bak*' -delete
	find ./src -name '*.sprite' -delete
	find . -name '*.dfs' -delete
	find . -name '*.raw' -delete
	find . -name '*.z64' -delete
	find . -name '*.n64' -delete
	rm -rf build/
-include $(DEPS)

.PHONY: all clean)";

void generate_makefile_gen_cpp(std::string makefile_path, const char *rom_filename,
							   const char *rom_title) {
	FILE *filestream = fopen(makefile_path.c_str(), "w");

	fprintf(filestream, makefile_gen_cpp, rom_filename, rom_filename, rom_title, rom_filename,
			rom_filename);

	fclose(filestream);
}