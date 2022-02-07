#include "Libdragon.h"

#include "ThreadCommand.h"

void Libdragon::Init(const std::string& folder) {
	char command[500];
	snprintf(command, 500, "cd %s\nlibdragon init", folder.c_str());
	ThreadCommand::RunCommand(command);
}
void Libdragon::InitSync(const std::string& folder) {
	char command[500];
	snprintf(command, 500, "cd %s\nlibdragon init", folder.c_str());
	system(command);
}

void Libdragon::Build(const std::string& folder) {
	char command[500];
	snprintf(command, 500, "cd %s\nlibdragon make -j > build.log", folder.c_str());
	ThreadCommand::RunCommand(command);
}

void Libdragon::Clean(const std::string& folder) {
	char command[500];
	snprintf(command, 500, "cd %s\nlibdragon exec make clean > build.log", folder.c_str());
	ThreadCommand::RunCommand(command);
}

void Libdragon::CleanSync(const std::string& folder) {
	char command[500];
	snprintf(command, 500, "cd %s\nlibdragon exec make clean > build.log", folder.c_str());
	system(command);
}
