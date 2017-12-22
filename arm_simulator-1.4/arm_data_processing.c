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
	uint32_t opcode = get_bits(ins, 24, 21);
	uint32_t rd, rn, shifter, bit20, bit_S, bit_I;
	rn = get_bits(ins, 19, 16);
	shifter = get_bits(ins, 11, 0);
	rd = get_bits(ins, 15, 12);
	bit_S = get_bit(ins, 20);
	bit_I = get_bit(ins, 25);
	switch (opcode) {
		case 0b0000: // AND ; Logical AND ; Rd := Rn AND shifter_operand
			//page 158
		if (cond) {
				if (bit_I) { // 2nd operand is a constant 

				}
				else { // 2nd operand is from a register

				}
			}
			else
				return -1;
			break;

		case 0b0001: // EOR ; Logical Exclusive OR ; Rd := Rn EOR shifter_operand
		break;

		case 0b0010: // SUB ; Subtract ; Rd := Rn - shifter_operand
		break;

		case 0b0011: // RSB ; Reverse Subtract ; Rd := shifter_operand - Rn
		break;

		case 0b0100: // ADD; Add ; Rd := Rn + shifter_operand
			// check cond first, 
		if (#condpassed) {
			uint32_t op1, op2, res;
			op1 = arm_read_register(p, rn);
			op2 = arm_read_register(p, shifter);
			res = op1 + op2;
			arm_write_register(p, rd, res);
			if ((bit_S == 1) && (rd == 15)) {
				maybe_write_to_cpsr(p);
			}
			else if (bit_S == 1) {
				uint32_t cpsr = read_cpsr(p->reg);
				uint32_t bit31_rd = get_bit(rd, N);
				update_N_Flag(&cpsr, bit31_rd);
				update_Z_Flag(&cpsr, bit31_rd);
					// C Flag = CarryFrom(Rn + shifter_operand)
					// V Flag = OverflowFrom(Rn + shifter_operand)
				write_cpsr(p->reg, cpsr);
			}
			return 0;
		}
		else
			return -1;
		break;

		case 0b0101: // ADC ; Add with Carry ; Rd := Rn + shifter_operand + Carry Flag
		break;

		case 0b0110: // SBC ; Subtract with Carry ; Rd := Rn - shifter_operand - NOT(Carry Flag)
		break;

		case 0b0111: // RSC ; Reverse Subtract with Carry ; Rd := shifter_operand - Rn - NOT(Carry Flag)
		break;

		case 0b1000: // TST ; Test ; Update flags after Rn AND shifter_operand
		break;

		case 0b1001: // TEQ ; Test Equivalence ; Update flags after Rn EOR shifter_operand
		break;

		case 0b1010: // CMP ; Compare ; Update flags after Rn - shifter_operand
		break;

		case 0b1011: // CMN ; Compare Negated ; Update flags after Rn + shifter_operand
		break;

		case 0b1100: // ORR ; Logical (inclusive) OR ; Rd := Rn OR shifter_operand
		break;

		case 0b1101: // MOV ; Move ; Rd := shifter_operand (no first operand)
			if (#cond) { // see implementation of conditional codes
				arm_write_register(p, rd, shifter);
				if ((bit_S == 1) && (rd == 15)) {
					maybe_write_to_cpsr(p);
				}
				else if (bit_S == 1) {
					uint32_t cpsr = read_cpsr(p->reg);
					uint32_t bit31_rd = get_bit(rd, N);
					update_N_Flag(&cpsr, bit31_rd);
					update_Z_Flag(&cpsr, bit31_rd);
					// C Flag = shifter_carry_out
					// V Flag = unaffected
					write_cpsr(p->reg, cpsr);
				}
				return 0;
			}
			else // #cond failed
				return -1;
			break;

		case 0b1110: // BIC ; Bit Clear ; Rd := Rn AND NOT(shifter_operand)
		break;

		case 0b1111: // MVN ; Move Not ; Rd := NOT shifter_operand (no first operand)
		break;
	} 
	return UNDEFINED_INSTRUCTION;
}

int arm_data_processing_immediate_msr(arm_core p, uint32_t ins) {
	return UNDEFINED_INSTRUCTION;
}

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

int ConditionPassed(arm_core p, uint8_t cond)
{

	uint32_t flag = arm_read_cpsr(p);

	switch(cond){
		case 0b000: 
			if(get_bit(flag,30) == 1)
				return 1;
			break;
		case 0b001: 
			if(get_bit(flag,30) == 0)
				return 1;
			break;
		case 0b010: 
			if(get_bit(flag,29) == 1)
				return 1;
			break;
		case 0b011: 
			if(get_bit(flag,29) == 0)
				return 1;
			break;
		case 0b0100: 
			if(get_bit(flag,31) == 1)
				return 1;
			break;
		case 0b0101: 
			if(get_bit(flag,31) == 0)
				return 1;
			break;
		case 0b0110: 
			if(get_bit(flag,28) == 1)
				return 1;
			break;
		case 0b0111: 
			if(get_bit(flag,28) == 0)
				return 1;
			break;
		case 0b1000: 
			if((get_bit(flag,29) == 1)&&(get_bit(flag,30) == 0))
		return 1;
		break;
		case 0b1001: if((get_bit(flag,29) == 0)&&(get_bit(flag,30) == 1))
		return 1;
		break;
		case 0b1010: if(get_bit(flag,31) == get_bit(flag,28))
		return 1;
		break;
		case 0b1011: if(get_bit(flag,31) != get_bit(flag,28))
		return 1;
		break;
		case 0b1100: if((get_bit(flag,30) == 0)&&(get_bit(flag,31) == get_bit(flag,28)))
		return 1;
		break;
		case 0b1101: if((get_bit(flag,30) == 1) || (get_bit(flag,31) != get_bit(flag,28)))
		return 1;
		break;
		default:
			return -1;
			
			break
		return 0;
	}
} 

// NB: theres going to be repetition of code, depending on if immediate or just shift
