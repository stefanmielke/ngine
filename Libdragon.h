#pragma once

#include <string>

class Libdragon {
   public:
	static void Init(const std::string& folder, const std::string& libdragon_exe_folder);
	static void InitSync(const std::string& folder, const std::string& libdragon_exe_folder);
	static void Build(const std::string& folder, const std::string& libdragon_exe_folder);
	static void Clean(const std::string& folder, const std::string& libdragon_exe_folder);
	static void CleanSync(const std::string& folder, const std::string& libdragon_exe_folder);
};
