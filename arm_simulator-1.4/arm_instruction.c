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
#include "arm_instruction.h"
#include "arm_exception.h"
#include "arm_data_processing.h"
#include "arm_load_store.h"
#include "arm_branch_other.h"
#include "arm_constants.h"
#include "util.h"

static int arm_execute_instruction(arm_core p) {
	uint32_t opc, instr, bits26_27, res, arm_decode;
	res = arm_fetch(p, &instr);
	bits26_27 = get_bits(res, 27, 26);
	int bit25 = get_bit(res, 25);
	switch(bits26_27) {
		case Ob00: // data processing and miscellaneous instructions
			int bit23_24 = get_bits(res, 24, 23);
			int bit20 = get_bit(res, 20);
			
			if ((bit23_24 == 0b10) && (bit20 == 0)) {
				arm_decode = arm_miscellaneous(p, instr);
			}
			else {
				if (bit25 == 0){
					// verify this again. bit25(a.k.a I bit) distinguishes btw immediate
					// shifter operand and a register-based shifter operand
					// If bit25==0 && bit4==1 && bit7==1, the instruction is not a data-processing instruction,
					// but lies in the arithmetic or Load/Store instruction extension space:
					arm_decode = arm_data_processing_shift(p, instr);
				}
				else if (bit25 == 1) {
					arm_decode = arm_data_processing_immediate_msr(p, instr);
				}
			}
			break;

		case Ob01: //  load/store instructions
			arm_decode = arm_load_store(p, instr);
			break;		

		case Ob10: // branchement and load/store multiple
			if (bit25 == 0) {
				// load/store multiple
				arm_decode = arm_load_store_multiple(p, instr);
			}
			else {
				arm_decode = arm_branch(p, instr);
			}
			
			break;

		case 0b11: // divers (coprocessor), not implemented
			if (bit25 == 0)
				arm_decode = arm_coprocessor_load_store(p, instr);
			break;

	} 
    return 0;
}

int arm_step(arm_core p) {
    int result;

    result = arm_execute_instruction(p);
    if (result)
        arm_exception(p, result);
    return result;
}
