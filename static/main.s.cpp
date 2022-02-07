const char *main_s_c =
	R"(#include <stdio.h>
#include <libdragon.h>

#include <mem_pool.h>
#include <scene_manager.h>

void setup();
void tick();
void display();

int main(void) {
	setup();

	while (1) {
		tick();

		display();
	}
})";