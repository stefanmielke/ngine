const char *main_s_c =
	R"(#include <stdio.h>
#include <libdragon.h>

#include <mem_pool.h>
#include <scene_manager.h>

MemZone global_memory_pool;
MemZone scene_memory_pool;

SceneManager *scene_manager;

void setup();

int main(void) {
	setup();

	printf("Hello world from NGine!\n");

	while (1) {
		scene_manager_tick(scene_manager);

		static display_context_t disp = 0;
		while (!(disp = display_lock()))
			;
		scene_manager_display(scene_manager, disp);
		display_show(disp);
	}
})";