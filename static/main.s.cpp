const char *main_s_cpp =
	R"(
#include <stdio.h>
#include <libdragon.h>

void setup();

int main(void) {
	setup();

	printf("Hello world from NGine!\n");

	while (1) {
	}
}
)";