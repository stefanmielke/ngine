const char *game_s_h =
	R"(#include <libdragon.h>
#include <mem_pool.h>
#include <scene_manager.h>

extern MemZone global_memory_pool;
extern MemZone scene_memory_pool;

extern SceneManager *scene_manager;
)";