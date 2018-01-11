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

uint32_t logicalShift(uint32_t value, uint32_t shift, char direction);

/* Decoding functions for different classes of instructions */
int arm_data_processing_shift(arm_core p, uint32_t ins) {
	uint32_t bits4_6, bits7_11, operand2, rm, rs, rs_bits_0_7, rs_bits_0_4, shift_imm, val1, val2, cFlag;
	uint32_t *shifter_carry_out;
	bits4_6 = get_bits(ins, 6, 4);
	rm = get_bits(ins, 3, 0);
	rm = arm_read_register(p, rm);
	rs = get_bits(ins, 11, 8);
	bits7_11 = get_bits(ins, 11, 7);
	rs_bits_0_7 = get_bits(rs, 7, 0);
	rs_bits_0_4 = get_bits(rs, 4, 0);
	switch (bits4_6) {
		case 0b000: // LSL Immediate
			shift_imm = get_bits(ins, 11, 7);
			if (shift_imm == 0) { /* Register Operand */
				operand2 = rm;
				*shifter_carry_out = get_bit(arm_read_cpsr(p), C);
			}
			else { /* shift_imm > 0 */
				operand2 = logicalShift(rm, shift_imm, 'l');
				*shifter_carry_out = get_bit(rm, 32 - shift_imm);
			}
		break;

		case 0b001: // LSL Register
			// here, Rs is in bits 11 - 8 and bit7 == 0
			if (rs_bits_0_7 == 0) {
				operand2 = rm;
				*shifter_carry_out = get_bit(arm_read_cpsr(p), C);
			}
			else if (rs_bits_0_7 < 32) {
				operand2 = logicalShift(rm, rs_bits_0_7, 'l'); 
				*shifter_carry_out = get_bit(rm, 32 - rs_bits_0_7);
			}
			else if (rs_bits_0_7 == 32) {
				operand2 = 0; 
				*shifter_carry_out = get_bit(rm, 0);
			}
			else {
				operand2 = 0; 
				*shifter_carry_out = 0;
			}
		break;

		case 0b010: // LSR Immediate
			if (!shift_imm) {
				operand2 = 0; 
				*shifter_carry_out = get_bit(rm, 31);
			}
			else { 
				operand2 = logicalShift(rm, shift_imm, 'r'); 
				*shifter_carry_out  = get_bit(rm, shift_imm - 1); 
			}
		break;

		case 0b011: // LSR Register
			// here, Rs is in bits 11 - 8 and bit7 == 0
			if (rs_bits_0_7 == 0) {
				operand2 = rm; 
				*shifter_carry_out = get_bit(arm_read_cpsr(p), C);
			}
			else if (rs_bits_0_7 < 32) {
				operand2 = logicalShift(rm, rs_bits_0_7, 'r');
				*shifter_carry_out = get_bit(rm, rs_bits_0_7 - 1);
			}
			else if (rs_bits_0_7 == 32) {
				operand2 = 0; 
				*shifter_carry_out = get_bit(rm, 31);
			}
			else {
				operand2 = 0; 
				*shifter_carry_out = 0;
			}
		break;

		case 0b100: // ASR Immediate
			if (!shift_imm) {
				if (get_bit(rm, 31) == 0) {
					operand2 = 0; 
				}
				else  {
					operand2 = 0xFFFFFFFF; 
				}
				*shifter_carry_out = get_bit(rm, 31);
			}
			else {
				operand2 = asr(rm, shift_imm);
				*shifter_carry_out = get_bit(rm, shift_imm - 1); 
			}
		break;

		case 0b101: // ASR Register
			// here, Rs is in bits 11 - 8 and bit7 == 0
			if (rs_bits_0_7 == 0) {
				operand2 = rm; 
				*shifter_carry_out = get_bit(arm_read_cpsr(p), C);
			}
			else if (rs_bits_0_7 < 32) {
				operand2 = asr(rm, rs_bits_0_7); // shifter_carry_out = Rm[Rs[7:0] - 1]
				*shifter_carry_out = get_bit(rm, rs_bits_0_7 - 1);
			}
			else {
				if (get_bit(rm, 31) == 0)
					operand2 = 0; // shifter_carry_out = Rm[31]
				else 
					operand2 = 0xFFFFFFFF; // shifter_carry_out = Rm[31]
				*shifter_carry_out = get_bit(rm, 31);
			}
		break;

		case 0b110: // ROR Immediate
			// here, check bits 7 - 11 to differenciate btw ROR Extend and ROR Immediate
			// for ROR_Ex, bits 7 - 11 == 0b00000
			cFlag = get_bit(get_bit(ins, 16), C);
			val1 = logicalShift(cFlag, 31, 'l');
			val2 = logicalShift(rm, 1, 'r');

			if (!bits7_11) { // ROR Extend
				operand2 = val1 | val2; // carryout = rm[0]
				*shifter_carry_out = get_bit(rm, 0);
			}
			else { // ROR Immediate
				if (!shift_imm) {
					// see rotate right with extend
					operand2 = val1 | val2; // carryout = rm[0]
					*shifter_carry_out = get_bit(rm, 0);
				}
				else {
					operand2 = ror(rm, shift_imm); // shifter_carry_out = Rm[shift_imm - 1]		
					*shifter_carry_out = get_bit(rm, shift_imm - 1);	
				}
			}
		break;

		case 0b111: // ROR Register
			// here, Rs is in bits 11 - 8 and bit7 == 0
			if (rs_bits_0_7 == 0) {
				operand2 = rm; 
				*shifter_carry_out = get_bit(arm_read_cpsr(p), C);
			}
			else if (rs_bits_0_4 == 0) {
				operand2 = rm; // carryout is rm[31]
				*shifter_carry_out = get_bit(rm, 31);
			}
			else {
				operand2 = ror(rm, rs_bits_0_4); // shifter_carry_out = Rm[Rs[4:0] - 1]
				*shifter_carry_out = get_bit(rm, rs_bits_0_4 - 1);
			}
		break;

		default:
			printf("Invalid data processing shift operation is being executed.!\n");
	}
	return operand2;
	// return UNDEFINED_INSTRUCTION;
}

int arm_data_processing_immediate_msr(arm_core p, uint32_t ins) {
	return UNDEFINED_INSTRUCTION;
}

uint32_t logicalShift(uint32_t value, uint32_t shift, char direction) {
	if (direction == 'r')
		return value >> shift;
	else if (direction == 'l')
		return value << shift;
	else 
		printf("Invalid shift direction argument! Possible unreliable result!\n");
	return 0;
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
