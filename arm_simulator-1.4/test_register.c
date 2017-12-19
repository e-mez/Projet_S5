#include "registers.c"
#include "arm_constants.c"
#include <stdio.h>

int main() {
	registers r = registers_create();
	uint32_t num = 10;
	write_register(r, 5, num); // num == 10 
	num = 100;
	write_register(r, 3, USR); // num == 10 
	printf("Register 5 contains %d\nRegister 3 contains %d\n", read_register(r, 5), read_register(r, 3));
	write_usr_register(r, 16, USR);
	printf("Mode: %d\n", get_mode(r));
	if (current_mode_has_spsr(r)) 
		printf("Has SPSR\n");
	else {
		printf("Reading usr_register : %d\n", read_usr_register(r, 3));
		printf("Reading usr_register : %d\n", read_usr_register(r, 16));
	}
	return 0;

}