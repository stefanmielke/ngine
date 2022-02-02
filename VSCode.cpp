#include "VSCode.h"

void VSCode::OpenFolder(std::string folder) {
	char cmd[255];
	snprintf(cmd, 255, "code %s", folder.c_str());
	system(cmd);
}
