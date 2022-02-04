#include "VSCode.h"

void VSCode::OpenPath(std::string path) {
	char cmd[255];
	snprintf(cmd, 255, "code %s", path.c_str());
	system(cmd);
}
