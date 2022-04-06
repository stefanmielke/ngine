#pragma once

#include <string>

class Libdragon {
   public:
	static void Init(const std::string& folder, const std::string& libdragon_exe_folder);
	static bool InitSync(const std::string& folder, const std::string& libdragon_exe_folder);
	static void Build(const std::string& folder, const std::string& libdragon_exe_folder);
	static void Clean(const std::string& folder, const std::string& libdragon_exe_folder);
	static void CleanSync(const std::string& folder, const std::string& libdragon_exe_folder);
	static void Update(const std::string& folder, const std::string& libdragon_exe_folder);
	static void Install(const std::string& folder, const std::string& libdragon_exe_folder);
};
