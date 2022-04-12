#include "generated.h"

const char *makefile_gen = R"(V=1
SOURCE_DIR=src
BUILD_DIR=build
include $(N64_INST)/include/n64.mk
include Makefile_custom.mk

N64_ROM_SAVETYPE = %s
N64_ROM_REGIONFREE = %s
N64_ROM_RTC = %s

N64_CFLAGS += -Ilibs/libdragon-extensions/include -fdiagnostics-color=never

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
include Makefile_custom.mk

N64_CFLAGS += -Ilibs/libdragon-extensions/include -fdiagnostics-color=never

N64_ROM_TITLE = "%s"
N64_ROM_SAVETYPE = %s
N64_ROM_REGIONFREE = %s
N64_ROM_RTC = %s

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

void generate_makefile_gen(const Project &project) {
	std::string makefile_path(project.project_settings.project_directory + "/Makefile");
	const char *rom_title = project.project_settings.project_name.c_str();
	const char *rom_filename = project.project_settings.rom_name.c_str();
	bool has_content = !project.images.empty() || !project.sounds.empty() ||
					   !project.general_files.empty();

	FILE *filestream = fopen(makefile_path.c_str(), "w");

	const char *save_type_items[] = {"none",	 "eeprom4k", "eeprom16", "sram256k",
									 "sram768k", "sram1m",	 "flashram"};
	const char *save_type_text = save_type_items[project.project_settings.save_type];
	const char *region_free_text = project.project_settings.region_free ? "true" : "false";
	const char *rtc_enabled_text = project.project_settings.modules.rtc ? "true" : "false";

	if (has_content) {
		fprintf(filestream, makefile_gen_content, rom_title, save_type_text, region_free_text,
				rtc_enabled_text, rom_filename, rom_filename, rom_filename, rom_filename,
				rom_filename);
	} else {
		fprintf(filestream, makefile_gen, save_type_text, region_free_text, rtc_enabled_text,
				rom_filename, rom_filename, rom_title, rom_filename, rom_filename);
	}

	fclose(filestream);
}