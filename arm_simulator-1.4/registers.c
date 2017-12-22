/*
Armator - simulateur de jeu d'instruction ARMv5T � but p�dagogique
Copyright (C) 2011 Guillaume Huard
Ce programme est libre, vous pouvez le redistribuer et/ou le modifier selon les
termes de la Licence Publique G�n�rale GNU publi�e par la Free Software
Foundation (version 2 ou bien toute autre version ult�rieure choisie par vous).

Ce programme est distribu� car potentiellement utile, mais SANS AUCUNE
GARANTIE, ni explicite ni implicite, y compris les garanties de
commercialisation ou d'adaptation dans un but sp�cifique. Reportez-vous � la
Licence Publique G�n�rale GNU pour plus de d�tails.

Vous devez avoir re�u une copie de la Licence Publique G�n�rale GNU en m�me
temps que ce programme ; si ce n'est pas le cas, �crivez � la Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307,
�tats-Unis.

Contact: Guillaume.Huard@imag.fr
	 B�timent IMAG
	 700 avenue centrale, domaine universitaire
	 38401 Saint Martin d'H�res
*/
#include "registers.h"
#include "arm_constants.h"
#include <stdlib.h>

struct registers_data {
	uint32_t *r_data; 
};

registers registers_create() {
    //registers r = NULL;
    registers r = malloc(sizeof(struct registers_data));
    r->r_data = malloc(sizeof(uint32_t) * 18); // 17 cases pour les 17 registres de base
    return r;
}

void registers_destroy(registers r) {
	free(r->r_data);
	free(r);
}

uint8_t get_mode(registers r) {
	// la mode est dans les bits 0-4 du CPSR (qui lui meme est dans l'indice 1 du tableau de registres)

	uint8_t mode = (uint8_t) (r->r_data[16] & 0b11111);
    return mode;
    // TO DO: v�rifier que le boutisme ne posera pas probl�me
} 

int current_mode_has_spsr(registers r) {
	uint8_t mode = get_mode(r);
	if ((mode == USR) || (mode == SYS))
		return 0;
	return 1;
}

int in_a_privileged_mode(registers r) {
    return get_mode(r) != USR;
}

uint32_t read_register(registers r, uint8_t reg) {
    uint32_t value=0;
    value = r->r_data[reg]; // lit le registre � l'indice reg
    return value;
}

uint32_t read_usr_register(registers r, uint8_t reg) {
    uint32_t value=0;
    if (!in_a_privileged_mode(r)) // USR est la seule mode qui n'est pas privileg� 
    	value = r->r_data[reg];
    return value;
}

uint32_t read_cpsr(registers r) {
    uint32_t value=0;
    value = r->r_data[16];
    return value;
}

uint32_t read_spsr(registers r) {
    uint32_t value=0;
    if (current_mode_has_spsr(r))
    	value = r->r_data[17];
    return value;
}

void write_register(registers r, uint8_t reg, uint32_t value) {
	r->r_data[reg] = value;
}

void write_usr_register(registers r, uint8_t reg, uint32_t value) {
	if (!in_a_privileged_mode(r))
		r->r_data[reg] = value;
}

void write_cpsr(registers r, uint32_t value) {
	r->r_data[16] = value;
}

void write_spsr(registers r, uint32_t value) {
	if (current_mode_has_spsr(r))
    	r->r_data[17] = value;
}
