#include "Libdragon.h"

#include <thread>

void Libdragon::Init(std::string folder) {
	char command[500];
	snprintf(command, 500, "cd %s\nlibdragon init", folder.c_str());
	std::thread(system, command).detach();
}

void Libdragon::Build(std::string folder) {
	char command[500];
	snprintf(command, 500, "cd %s\nlibdragon make -j", folder.c_str());
	std::thread(system, command).detach();
}

void Libdragon::Rebuild(std::string folder) {
	char command[500];
	snprintf(command, 500, "cd %s\nlibdragon make clean\nlibdragon make -j", folder.c_str());
	std::thread(system, command).detach();
}
