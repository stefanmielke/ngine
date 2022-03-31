#pragma once

#include <string>

extern char separator[];

class ThreadCommand {
   public:
	static void QueueCommand(std::string command);
	static void RunCommand(std::string command);
	static void RunCommandDetached(std::string command);
};
