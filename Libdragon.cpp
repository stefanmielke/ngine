#include "Libdragon.h"

#include "ThreadCommand.h"

void Libdragon::Init(const std::string &folder, const std::string &libdragon_exe_folder) {
	char command[500];
	snprintf(command, 500, "%s init", libdragon_exe_folder.c_str());
	ThreadCommand::QueueCommand(command);
}
bool Libdragon::InitSync(const std::string &folder, const std::string &libdragon_exe_folder) {
	char command[500];
	snprintf(command, 500, "%s init", libdragon_exe_folder.c_str());
	return ThreadCommand::RunCommand(command) == EXIT_SUCCESS;
}

void Libdragon::Build(const std::string &folder, const std::string &libdragon_exe_folder) {
	char command[500];
	snprintf(command, 500, "%s make -j", libdragon_exe_folder.c_str());
	ThreadCommand::QueueCommand(command);

	ThreadCommand::QueueCommand("echo Build Completed.");
}

void Libdragon::Clean(const std::string &folder, const std::string &libdragon_exe_folder) {
	char command[500];
	snprintf(command, 500, "%s exec make clean", libdragon_exe_folder.c_str());
	ThreadCommand::QueueCommand(command);
}

void Libdragon::CleanSync(const std::string &folder, const std::string &libdragon_exe_folder) {
	char command[500];
	snprintf(command, 500, "%s exec make clean", libdragon_exe_folder.c_str());
	ThreadCommand::RunCommand(command);
}

void Libdragon::Update(const std::string &folder, const std::string &libdragon_exe_folder) {
	char command[500];
	snprintf(command, 500, "%s update", libdragon_exe_folder.c_str());
	ThreadCommand::QueueCommand(command);
}

void Libdragon::Install(const std::string &folder, const std::string &libdragon_exe_folder) {
	char command[500];
	snprintf(command, 500, "%s install", libdragon_exe_folder.c_str());
	ThreadCommand::QueueCommand(command);
}

void Libdragon::Disasm(const std::string &folder, const std::string &libdragon_exe_folder) {
	char command[500];
	snprintf(command, 500, "%s disasm > rom.asm", libdragon_exe_folder.c_str());
	ThreadCommand::QueueCommand(command);
}

std::string Libdragon::GetVersion(const std::string &libdragon_exe_folder) {
	char command[500];
	snprintf(command, 500, "%s version", libdragon_exe_folder.c_str());

	std::string result;
	if (ThreadCommand::RunCommand(command, result) != EXIT_SUCCESS) {
		return "Update Libdragon-CLI to see version";
	} else {
		return result;
	}
}

void Libdragon::GitCheckout(const std::string &libdragon_exe_folder,
							const std::string &checkout_folder_relative,
							const std::string &branch) {
	char cmd[255];
	snprintf(cmd, 255, "%s exec bash -c 'cd %s; git checkout %s'", libdragon_exe_folder.c_str(),
			 checkout_folder_relative.c_str(), branch.c_str());
	ThreadCommand::QueueCommand(cmd);
}

bool Libdragon::GitSubmoduleAddSync(const std::string &libdragon_exe_folder,
									const std::string &submodule_uri,
									const std::string &submodule_folder) {
	char cmd[500];
	snprintf(cmd, 500, "%s exec git submodule add %s %s", libdragon_exe_folder.c_str(),
			 submodule_uri.c_str(), submodule_folder.c_str());

	return ThreadCommand::RunCommand(cmd) == EXIT_SUCCESS;
}
