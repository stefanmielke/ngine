const char *gitignore =
	R"(
.vscode/**
!.vscode/launch.json
!.vscode/tasks.json
!.vscode/c_cpp_properties.json

build/
build.log

*.v64
*.z64
*.elf
*.o
*.bin
*.dfs
*.d

filesystem/

/src/res/

*.plan_bak*

# cmake files
cmake-*/

# removing csv files generated during build-resources.sh
/assets/maps/**/*.csv
)";