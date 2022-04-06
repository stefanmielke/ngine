#pragma once

#include <string>

extern char separator[];

class ThreadCommand {
   public:
	static void QueueCommand(std::string command);
	static int RunCommand(std::string command);
	static void RunCommandDetached(std::string command);
};
