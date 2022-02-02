const char *gitignore =
	".vscode/**\n"
	"!.vscode/launch.json\n"
	"!.vscode/tasks.json\n"
	"!.vscode/c_cpp_properties.json\n"
	"\n"
	"build/\n"
	"\n"
	"*.v64\n"
	"*.z64\n"
	"*.elf\n"
	"*.o\n"
	"*.bin\n"
	"*.dfs\n"
	"*.d\n"
	"\n"
	"filesystem/\n"
	"\n"
	"/src/res/\n"
	"\n"
	"*.plan_bak*\n"
	"\n"
	"# cmake files\n"
	"cmake-*/\n"
	"\n"
	"# removing csv files generated during build-resources.sh\n"
	"/assets/maps/**/*.csv";