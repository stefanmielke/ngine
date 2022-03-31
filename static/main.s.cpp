const char *main_s_c =
	R"(void setup();
void tick();
void display();

int main(void) {
	setup();

	while (1) {
		tick();

		display();
	}
})";