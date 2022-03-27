#include "Libdragon.h"

#include "ThreadCommand.h"

void Libdragon::Init(const std::string& folder, const std::string& libdragon_exe_folder) {
	char command[500];
	snprintf(command, 500, "cd %s\n%s init", folder.c_str(), libdragon_exe_folder.c_str());
	ThreadCommand::RunCommand(command);
}
void Libdragon::InitSync(const std::string& folder, const std::string& libdragon_exe_folder) {
	char command[500];
	snprintf(command, 500, "cd %s\n%s init", folder.c_str(), libdragon_exe_folder.c_str());
	system(command);
}

void Libdragon::Build(const std::string& folder, const std::string& libdragon_exe_folder) {
	char command[500];
	snprintf(command, 500, "cd %s\n%s make -j > build.log", folder.c_str(), libdragon_exe_folder.c_str());
	ThreadCommand::RunCommand(command);
}

void Libdragon::Clean(const std::string& folder, const std::string& libdragon_exe_folder) {
	char command[500];
	snprintf(command, 500, "cd %s\n%s exec make clean > build.log", folder.c_str(), libdragon_exe_folder.c_str());
	ThreadCommand::RunCommand(command);
}

void Libdragon::CleanSync(const std::string& folder, const std::string& libdragon_exe_folder) {
	char command[500];
	snprintf(command, 500, "cd %s\n%s exec make clean > build.log", folder.c_str(), libdragon_exe_folder.c_str());
	system(command);
}

void Libdragon::Update(const std::string &folder, const std::string &libdragon_exe_folder) {
	char command[500];
	snprintf(command, 500, "cd %s\n%s update > build.log", folder.c_str(), libdragon_exe_folder.c_str());
	ThreadCommand::RunCommand(command);
}

void Libdragon::Install(const std::string &folder, const std::string &libdragon_exe_folder) {
	char command[500];
	snprintf(command, 500, "cd %s\n%s install > build.log", folder.c_str(), libdragon_exe_folder.c_str());
	ThreadCommand::RunCommand(command);
}
