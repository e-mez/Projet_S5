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

	int ConditionPassed(arm_core p, uint8_t cond);

	static int arm_execute_instruction(arm_core p) {
		uint32_t opcode, operand1, operand2, operand3, operands_result, rd, instr, bit25, bit23_24, bits26_27, bits4_11, res, arm_decode;
		uint32_t load_store_status;
		res = arm_fetch(p, &instr);
		bits26_27 = get_bits(res, 27, 26);
		bit25 = get_bit(res, 25);
		opcode = get_bits(res, 24, 21);
		operand1 = get_bits(res, 19, 16);
		operand1 = arm_read_register(p, operand1);
		rd = get_bits(res, 15, 12);
		rd = arm_read_register(p, rd);
		uint8_t cond_bits = get_bits(res, 31, 28);
		switch(bits26_27) {
			case 0b00: /* data processing and miscellaneous instructions */
				bit23_24 = get_bits(res, 24, 23);
				int bit20 = get_bit(res, 20);
				
				if ((bit23_24 == 0b10) && (bit20 == 0)) {
					arm_decode = arm_miscellaneous(p, instr);
				}
				else { // data pro starts here
					if (ConditionPassed(p, cond_bits)) {
						if (bit25 == 0){ // rn is not immediate operand
							if (!bits4_11) { // register operand value
								uint32_t bits0_3 = get_bits(res, 3, 0);
								operand2 = arm_read_register(p, bits0_3);
							}
							else { // shifted register operand value
							}
						
						}
						else { // rn is immediate operand
							int bit8_11, bit0_7;
							bit8_11 = get_bits(res, 11, 8);
							bit0_7 = get_bits(res, 7, 0);
							operand2 = ror(bit0_7, bit8_11 * 2);
						}
						// here, u have the operands and the opcode. time to switch on the opcode and code the corresponding instruction
						switch(opcode) {
							case 0b0000: // AND ; Logical AND ; Rd := Rn AND shifter_operand
								operands_result = operand1 & operand2;
								arm_write_register(p, rd, operands_result);
								break;

							case 0b0001: // EOR ; Logical Exclusive OR ; Rd := Rn EOR shifter_operand
								operands_result = operand1 ^ operand2;
								arm_write_register(p, rd, operands_result);
								break;

							case 0b0010: // SUB ; Subtract ; Rd := Rn - shifter_operand
								operands_result = operand1 - operand2;
								arm_write_register(p, rd, operands_result);
								break;

							case 0b0011: // RSB ; Reverse Subtract ; Rd := shifter_operand - Rn
								operands_result = operand2 - operand1;
								arm_write_register(p, rd, operands_result);
								break;

							case 0b0100: // ADD; Add ; Rd := Rn + shifter_operand
								operands_result = operand1 + operand2;
								arm_write_register(p, rd, operands_result);
								break;

							case 0b0101: // ADC ; Add with Carry ; Rd := Rn + shifter_operand + Carry Flag
								operand3 = get_bit(res, C);
								operands_result = operand1 + operand2 + operand3;
								arm_write_register(p, rd, operands_result);
								break;

							case 0b0110: // SBC ; Subtract with Carry ; Rd := Rn - shifter_operand - NOT(Carry Flag)
								operand3 = get_bit(res, C);
								if (!operand3) operand3 = 1;
								else operand3 = 0;
								operands_result = operand1 - operand2 - operand3;
								arm_write_register(p, rd, operands_result);
								break;

							case 0b0111: // RSC ; Reverse Subtract with Carry ; Rd := shifter_operand - Rn - NOT(Carry Flag)
								operand3 = get_bit(res, C);
								if (!operand3) operand3 = 1;
								else operand3 = 0;
								operands_result = operand2 - operand1 - operand3;
								arm_write_register(p, rd, operands_result);
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
								operands_result = operand1 | operand2;
								arm_write_register(p, rd, operands_result);
								break;

							case 0b1101: // MOV ; Move ; Rd := shifter_operand (no first operand)
								arm_write_register(p, rd, operand2);
								break;

							case 0b1110: // BIC ; Bit Clear ; Rd := Rn AND NOT(shifter_operand)
								operands_result = operand1 & (!operand2);
								arm_write_register(p, rd, operands_result);
								break;

							case 0b1111: // MVN ; Move Not ; Rd := NOT shifter_operand (no first operand)
								arm_write_register(p, rd, !operand2);
								break;
						}
					}
					else { // Condition not passed
						printf("Condition not passed (bits 28 - 31)\n");
					}
					
				}
				break;

			case 0b01: //  load/store instructions
				if (ConditionPassed(p,cond_bits)) {
					load_store_status = arm_load_store(p, instr);
				}
				
				break;		

			case 0b10: // branchement and load/store multiple
				if (bit25 == 0) {
					// load/store multiple
					arm_decode = arm_load_store_multiple(p, instr);
				}
				else {
					if (ConditionPassed(p,cond_bits)) {
						arm_decode = arm_branch(p, instr);
					}
					
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

	int ConditionPassed(arm_core p, uint8_t cond) {

		uint32_t flag = arm_read_cpsr(p);

		switch(cond) {
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
			case 0b1001: 
				if((get_bit(flag,29) == 0)&&(get_bit(flag,30) == 1))
					return 1;
			break;
			case 0b1010: 
				if(get_bit(flag,31) == get_bit(flag,28))
					return 1;
				break;
			case 0b1011: 
				if(get_bit(flag,31) != get_bit(flag,28))
					return 1;
				break;
			case 0b1100: 
				if((get_bit(flag,30) == 0)&&(get_bit(flag,31) == get_bit(flag,28)))
					return 1;
				break;
			case 0b1101: 
				if((get_bit(flag,30) == 1) || (get_bit(flag,31) != get_bit(flag,28)))
					return 1;
				break;
			default:
				return -1;			
				break;

			return 0;
		}
	}
