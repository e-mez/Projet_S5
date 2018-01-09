/*
Armator - simulateur de jeu d'instruction ARMv5T à but pédagogique
Copyright (C) 2011 Guillaume Huard
Ce programme est libre, vous pouvez le redistribuer et/ou le modifier selon les
termes de la Licence Publique Générale GNU publiée par la Free Software
Foundation (version 2 ou bien toute autre version ultérieure choisie par vous).

Ce programme est distribué car potentiellement utile, mais SANS AUCUNE
GARANTIE, ni explicite ni implicite, y compris les garanties de
commercialisation ou d'adaptation dans un but spécifique. Reportez-vous à la
Licence Publique Générale GNU pour plus de détails.

Vous devez avoir reçu une copie de la Licence Publique Générale GNU en même
temps que ce programme ; si ce n'est pas le cas, écrivez à la Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307,
États-Unis.

Contact: Guillaume.Huard@imag.fr
	 Bâtiment IMAG
	 700 avenue centrale, domaine universitaire
	 38401 Saint Martin d'Hères
*/
#include "arm_data_processing.h"
#include "arm_exception.h"
#include "arm_constants.h"
#include "arm_branch_other.h"
#include "util.h"
#include "debug.h"
#include "arm_core.h"

/* Decoding functions for different classes of instructions */
int arm_data_processing_shift(arm_core p, uint32_t ins) {
	
		 
	return UNDEFINED_INSTRUCTION;
}

int arm_data_processing_immediate_msr(arm_core p, uint32_t ins) {
	return UNDEFINED_INSTRUCTION;
}



/*
void update_N_Flag(uint32_t *cpsr, uint32_t rd31){
	if (rd31)
		set_bit(*cpsr, N);
	else
		clr_bit(*cpsr, N);
}

void update_Z_Flag(uint32_t *cpsr, uint32_t rd31){
	if (!rd31)
		set_bit(*cpsr, Z);
	else 
		clr_bit(*cpsr, Z);
}

void maybe_write_to_cpsr(arm_core p) {
	if (current_mode_has_spsr(p->reg))
		arm_write_register(p, 16, 17);
	else 
		exit(1); // UNPREDICTABLE	
}

*/
