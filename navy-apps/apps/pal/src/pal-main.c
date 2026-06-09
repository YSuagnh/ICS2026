#include <common.h>
#include <FLOAT.h>

void main_loop();
void hal_init();

int
main(void) {
	Log("game start!");
	Log("%d\n", f2F(9));
	Log("%d\n", Fsqrt(f2F(9)));
	Log("%d\n", Fpow(f2F(10),f2F(0.333)));

  hal_init();
	main_loop();

	return 0;
}
