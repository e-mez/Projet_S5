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
#include "arm_load_store.h"
#include "arm_exception.h"
#include "arm_constants.h"
#include "util.h"
#include "debug.h"

uint32_t getValueFromIns(arm_core p, uint32_t ins, int h, int l);
void load_store_shifts_rotate(uint32_t *shift, uint32_t *shift_imm, uint32_t *index, uint32_t *rm, uint32_t *ins);

int arm_load_store(arm_core p, uint32_t ins) { // return 1 if OK, UNDEFINED_INSTRUCTION if not
	uint32_t bit25, bit21, bit23, rn, rd, rm, offset12, address, bits4_11, bit24, shift, shift_imm, index;
	rn = getValueFromIns(p, ins, 19, 16);
	rd = getValueFromIns(p, ins, 15, 12);
	rm = getValueFromIns(p, ins, 3, 0);
	bit25 = get_bit(ins, 25);
	bit21 = get_bit(ins, 21);
	bit23 = get_bit(ins, 23); // getting the U bit
	bit24 = get_bit(ins, 24);
	//bits26_27 = get_bits(27, 26);
	shift_imm = get_bits(ins, 11, 7);
	shift = get_bits(ins, 6, 5);
	if (bit25) { // Register offset/index or Scaled Register offset/index
		bits4_11 = get_bits(ins, 11, 4);
		if (bits4_11) { // Scaled register offset/index
			if (bit24) { // Scaled register pre-indexed
				load_store_shifts_rotate(&shift, &shift_imm, &index, &rm, &ins);
				if (bit23) 
					rn = rn + index;
				else
					rn = rn - index;
				address = rn;
				//return 1;
			}
			else { // Scaled register post-indexed
				address = rn;
				load_store_shifts_rotate(&shift, &shift_imm, &index, &rm, &ins);				
				if (bit23) 
					rn = rn + index;
				else
					rn = rn - index;
				//return 1;
			}
		}
		else { // Register offset/index
			if (bit24) { // Register pre-indexed
				if (bit23)
					address = rn + rm;			
				else 
					address = rn - rm;
				rn = address;
			}
			else { // Register post-indexed
				address = rn;
				if (bit23)
					rn = rn + rm;			
				else 
					rn = rn - rm;
			}
		}
		//return 1;
	}
	else { // immediate offset/index
		offset12 = get_bits(ins, 11, 0);
		if (bit21) { // Immediate pre-indexed
			if (bit23)
				address = rn + offset12;			
			else 
				address = rn - offset12;
			rn = address;
		}
		else { // Immediate offset post-indexed
			address = rn;
			if (bit23)
				rn = rn + offset12;			
			else 
				rn = rn - offset12;
		}
		//return 1;
		// u need to return the rn !!!!!
	}
	//printf("i am here\n");
	return rn;
    //return UNDEFINED_INSTRUCTION;
}

int arm_load_store_multiple(arm_core p, uint32_t ins) {
    return UNDEFINED_INSTRUCTION;
}

int arm_coprocessor_load_store(arm_core p, uint32_t ins) {
    /* Not implemented */
    return UNDEFINED_INSTRUCTION;
}

uint32_t getValueFromIns(arm_core p, uint32_t ins, int h, int l) {
	uint32_t value;
	value = get_bits(ins, h, l);
	//value = arm_read_register(p, value);
	return value;
}

void load_store_shifts_rotate(uint32_t *shift, uint32_t *shift_imm, uint32_t *index, uint32_t *rm, uint32_t *ins) {
	switch (*shift) {
		case 0b00: // Logical Shift Left
			*index = (*rm) << (*shift_imm);
		break;

		case 0b01: // Logical Shift Right
			if (*shift_imm == 0)  /* LSR #32 */
				*index = 0;
			else
				*index = (*rm) >> (*shift_imm);
		break;

		case 0b10: // Arithmetic Shift Right
			if (*shift_imm == 0) { /* ASR #32 */
				if (get_bit(*rm, 31)) 
					*index = 0xFFFFFFFF;
				else
					*index = 0;
			}
			else
				*index = asr(*rm, *shift_imm);
		break;

		case 0b11: // ROR or RRX
			if (*shift_imm == 0) /* RRX */
				*index = (get_bit(*ins, C) << 31) || (*rm >> 1);
			else /* ROR */
				*index = ror(*rm, *shift_imm);
		break;
	}
}
