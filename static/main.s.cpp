const char *main_s_cpp =
	R"(
#include <stdio.h>
#include <libdragon.h>

int main(void) {
	console_init();

	debug_init_usblog();
	console_set_debug(true);

	printf("Hello world from NGine!\n");

	while (1) {
	}
}
)";