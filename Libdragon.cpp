#include "Libdragon.h"

#include "ThreadCommand.h"

void Libdragon::Init(const std::string &folder, const std::string &libdragon_exe_folder) {
	char command[500];
	snprintf(command, 500, "cd %s%s%s init", folder.c_str(), separator,
			 libdragon_exe_folder.c_str());
	ThreadCommand::RunCommand(command);
}
void Libdragon::InitSync(const std::string &folder, const std::string &libdragon_exe_folder) {
	char command[500];
	snprintf(command, 500, "cd %s%s%s init", folder.c_str(), separator,
			 libdragon_exe_folder.c_str());
	system(command);
}

void Libdragon::Build(const std::string &folder, const std::string &libdragon_exe_folder) {
	char command[500];
	snprintf(command, 500, "cd %s%s%s make -j", folder.c_str(), separator,
			 libdragon_exe_folder.c_str());
	ThreadCommand::RunCommand(command);
}

void Libdragon::Clean(const std::string &folder, const std::string &libdragon_exe_folder) {
	char command[500];
	snprintf(command, 500, "cd %s%s%s exec make clean", folder.c_str(), separator,
			 libdragon_exe_folder.c_str());
	ThreadCommand::RunCommand(command);
}

void Libdragon::CleanSync(const std::string &folder, const std::string &libdragon_exe_folder) {
	char command[500];
	snprintf(command, 500, "cd %s%s%s exec make clean", folder.c_str(), separator,
			 libdragon_exe_folder.c_str());
	system(command);
}

void Libdragon::Update(const std::string &folder, const std::string &libdragon_exe_folder) {
	char command[500];
	snprintf(command, 500, "cd %s%s%s update", folder.c_str(), separator,
			 libdragon_exe_folder.c_str());
	ThreadCommand::RunCommand(command);
}

void Libdragon::Install(const std::string &folder, const std::string &libdragon_exe_folder) {
	char command[500];
	snprintf(command, 500, "cd %s%s%s install", folder.c_str(), separator,
			 libdragon_exe_folder.c_str());
	ThreadCommand::RunCommand(command);
}
