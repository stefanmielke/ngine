#include "generated.h"

const char *makefile_gen = R"(V=1
SOURCE_DIR=src
BUILD_DIR=build
include $(N64_INST)/include/n64.mk

N64_CFLAGS += -Ilibs/libdragon-extensions/include

C_ROOT_FILES := $(wildcard src/*.c)
C_ROOT_1_FILES := $(wildcard src/**/*.c)
C_ROOT_2_FILES := $(wildcard src/**/**/*.c)
C_ROOT_3_FILES := $(wildcard src/**/**/**/*.c)
C_ROOT_4_FILES := $(wildcard src/**/**/**/**/*.c)
C_ROOT_5_FILES := $(wildcard src/**/**/**/**/**/*.c)
C_LIB_EXTENSIONS_FILES := $(wildcard libs/libdragon-extensions/src/*.c)

SRC = $(C_ROOT_FILES) $(C_ROOT_1_FILES) $(C_ROOT_2_FILES) $(C_ROOT_3_FILES) $(C_ROOT_4_FILES) $(C_ROOT_5_FILES) $(C_LIB_EXTENSIONS_FILES)
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
	find . -name '*.gen.c' -delete
	find . -name '*.gen.h' -delete
	rm -rf build/

-include $(DEPS)

.PHONY: all clean)";

const char *makefile_gen_content = R"(V=1
SOURCE_DIR=src
BUILD_DIR=build
include $(N64_INST)/include/n64.mk

N64_CFLAGS += -Ilibs/libdragon-extensions/include

N64_ROM_TITLE = "%s"

C_ROOT_FILES := $(wildcard src/*.c)
C_ROOT_1_FILES := $(wildcard src/**/*.c)
C_ROOT_2_FILES := $(wildcard src/**/**/*.c)
C_ROOT_3_FILES := $(wildcard src/**/**/**/*.c)
C_ROOT_4_FILES := $(wildcard src/**/**/**/**/*.c)
C_ROOT_5_FILES := $(wildcard src/**/**/**/**/**/*.c)
C_LIB_EXTENSIONS_FILES := $(wildcard libs/libdragon-extensions/src/*.c)

SRC = $(C_ROOT_FILES) $(C_ROOT_1_FILES) $(C_ROOT_2_FILES) $(C_ROOT_3_FILES) $(C_ROOT_4_FILES) $(C_ROOT_5_FILES) $(C_LIB_EXTENSIONS_FILES)
OBJS = $(SRC:%%.c=%%.o)
DEPS = $(SRC:%%.c=%%.d)

all: %s.z64
%s.z64: $(BUILD_DIR)/%s.dfs

$(BUILD_DIR)/%s.dfs: $(wildcard build/filesystem/*)
	$(N64_MKDFS) $@ build/filesystem

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
	find . -name '*.gen.c' -delete
	find . -name '*.gen.h' -delete
	rm -rf build/

-include $(DEPS)

.PHONY: all clean)";

void generate_makefile_gen(std::string &makefile_path, const char *rom_filename,
						   const char *rom_title, bool has_content) {
	FILE *filestream = fopen(makefile_path.c_str(), "w");

	if (has_content) {
		fprintf(filestream, makefile_gen_content, rom_title, rom_filename, rom_filename,
				rom_filename, rom_filename, rom_filename);
	} else {
		fprintf(filestream, makefile_gen, rom_filename, rom_filename, rom_title, rom_filename,
				rom_filename);
	}

	fclose(filestream);
}