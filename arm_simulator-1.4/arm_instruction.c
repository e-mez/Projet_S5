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
	void loadStoreByteOrWord(arm_core *p, uint32_t load_store_address, uint32_t rd_value, uint32_t bit20, uint32_t bit22, uint32_t rd);
	uint32_t shiftedRegisterOperand(arm_core p, uint32_t ins, uint32_t *shifter_carry_out);
	uint32_t logicalShift(uint32_t value, uint32_t shift, char direction);
	void update_Flag_Z(arm_core p, int indice);
    void update_Flag_N(arm_core p, int indice);
    void update_Flag_C(arm_core p, int indice);
    void update_Flag_V(arm_core p, int indice);
    void update_flag(arm_core p, uint64_t operands_result);

	static int arm_execute_instruction(arm_core p) {
		uint32_t opcode, operand1, operand2, operand3, rd, instr, bit25, bit23_24, bits26_27, bits4_11, res, arm_decode;
		uint32_t shifter_carry_out, load_store_address, cpsr, bit20;
		uint64_t operands_result, alu_out;
		res = arm_fetch(p, &instr);
		bits26_27 = get_bits(res, 27, 26);
		bit25 = get_bit(res, 25);
		opcode = get_bits(res, 24, 21);
		operand1 = get_bits(res, 19, 16);
		operand1 = arm_read_register(p, operand1);
		rd = get_bits(res, 15, 12);
		//rd = arm_read_register(p, rd);
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
								shifter_carry_out = get_bit(arm_read_cpsr(p), C);
							}
							else { // shifted register operand value
								operand2 = shiftedRegisterOperand(p, res, &shifter_carry_out);
							}
						
						}
						else { // rn is immediate operand
							uint32_t bit8_11, bit0_7;
							bit8_11 = get_bits(res, 11, 8);
							bit0_7 = get_bits(res, 7, 0);
							operand2 = ror(bit0_7, bit8_11 * 2);
							if (bit8_11 == 0)
								shifter_carry_out = get_bit(arm_read_cpsr(p), C); 
							else /* rotate_imm != 0 */
								shifter_carry_out = get_bit(operand2, 31);
						}
						// here, u have the operands and the opcode. time to switch on the opcode and code the corresponding instruction
						switch(opcode) {
							case 0b0000: // AND ; Logical AND ; Rd := Rn AND shifter_operand
								operands_result = operand1 & operand2;
								arm_write_register(p, rd, (uint32_t) operands_result);
								update_flag(p,operands_result);
								break;

							case 0b0001: // EOR ; Logical Exclusive OR ; Rd := Rn EOR shifter_operand
								operands_result = operand1 ^ operand2;
								arm_write_register(p, rd, (uint32_t) operands_result);
								update_flag(p,operands_result);
								break;

							case 0b0010: // SUB ; Subtract ; Rd := Rn - shifter_operand
								operands_result = operand1 - operand2;
								arm_write_register(p, rd, (uint32_t) operands_result);
								update_flag(p,operands_result);
								break;

							case 0b0011: // RSB ; Reverse Subtract ; Rd := shifter_operand - Rn
								operands_result = operand2 - operand1;
								arm_write_register(p, rd, (uint32_t) operands_result);
								update_flag(p,operands_result);
								break;

							case 0b0100: // ADD; Add ; Rd := Rn + shifter_operand
								operands_result = operand1 + operand2;
								arm_write_register(p, rd, (uint32_t) operands_result);
								update_flag(p,operands_result);
								break;

							case 0b0101: // ADC ; Add with Carry ; Rd := Rn + shifter_operand + Carry Flag
								operand3 = get_bit(arm_read_cpsr(p), C);
								operands_result = operand1 + operand2 + operand3;
								arm_write_register(p, rd, (uint32_t) operands_result);
								update_flag(p,operands_result);
								break;

							case 0b0110: // SBC ; Subtract with Carry ; Rd := Rn - shifter_operand - NOT(Carry Flag)
								operand3 = get_bit(arm_read_cpsr(p), C);
								if (!operand3) operand3 = 1;
								else operand3 = 0;
								operands_result = operand1 - operand2 - operand3;
								arm_write_register(p, rd, (uint32_t) operands_result);
								update_flag(p,operands_result);
								break;

							case 0b0111: // RSC ; Reverse Subtract with Carry ; Rd := shifter_operand - Rn - NOT(Carry Flag)
								operand3 = get_bit(arm_read_cpsr(p), C);
								if (!operand3) operand3 = 1;
								else operand3 = 0;
								operands_result = operand2 - operand1 - operand3;
								arm_write_register(p, rd, (uint32_t) operands_result);
								update_flag(p,operands_result);
								break;

							case 0b1000: // TST ; Test ; Update flags after Rn AND shifter_operand
								// If the I bit is 0 and both bit[7] and bit[4] of shifter_operand are 1, the instruction is not TST.
								alu_out = operand1 & operand2;
								update_Flag_Z(p, !alu_out);
								update_Flag_N(p, get_bit(alu_out, 31));
								update_Flag_C(p, shifter_carry_out);
								/*
									N Flag = alu_out[31]
Z Flag = if alu_out == 0 then 1 else 0
C Flag = shifter_carry_out
V Flag = unaffected
								*/
							break;

							case 0b1001: // TEQ ; Test Equivalence ; Update flags after Rn EOR shifter_operand
								alu_out = operand1 | operand2;
								update_Flag_Z(p, !alu_out);
								update_Flag_N(p, get_bit(alu_out, 31));
								update_Flag_C(p, shifter_carry_out);
								/*
N Flag = alu_out[31]
Z Flag = if alu_out == 0 then 1 else 0
C Flag = shifter_carry_out
V Flag = unaffected
*/
								break;

							case 0b1010: // CMP ; Compare ; Update flags after Rn - shifter_operand
								alu_out = operand1 - operand2;
								update_Flag_Z(p, !alu_out);
								update_Flag_N(p, get_bit(alu_out, 31));
								if(get_bits(alu_out,63,32)!=0b0) { //dans ce cas y'a retenue et overflow
						            update_Flag_V(p,1);
						         	update_Flag_C(p,1);
						     	}
/*
N Flag = alu_out[31]
Z Flag = if alu_out == 0 then 1 else 0
C Flag = NOT BorrowFrom(Rn - shifter_operand)
V Flag = OverflowFrom(Rn - shifter_operand)
*/
								break;

							case 0b1011: // CMN ; Compare Negated ; Update flags after Rn + shifter_operand
								alu_out = operand1 + operand2;
								update_Flag_Z(p, !alu_out);
								update_Flag_N(p, get_bit(alu_out, 31));
								if(get_bits(alu_out,63,32)!=0b0) { //dans ce cas y'a retenue et overflow
						            update_Flag_V(p,1);
						         	update_Flag_C(p,1);
						     	}
/*
N Flag = alu_out[31]
Z Flag = if alu_out == 0 then 1 else 0
C Flag = CarryFrom(Rn + shifter_operand)
V Flag = OverflowFrom(Rn + shifter_operand)
*/
								break;

							case 0b1100: // ORR ; Logical (inclusive) OR ; Rd := Rn OR shifter_operand
								operands_result = operand1 | operand2;
								//cpsr = arm_read_cpsr(p);
								update_flag(p,operands_result);
								arm_write_register(p, rd, (uint32_t) operands_result);
								break;

							case 0b1101: // MOV ; Move ; Rd := shifter_operand (no first operand)
								arm_write_register(p, rd, (uint32_t) operand2);
								break;

							case 0b1110: // BIC ; Bit Clear ; Rd := Rn AND NOT(shifter_operand)
								operands_result = operand1 & (!operand2);
								arm_write_register(p, rd, (uint32_t) operands_result);
								update_flag(p,operands_result);
								break;

							case 0b1111: // MVN ; Move Not ; Rd := NOT shifter_operand (no first operand)
								arm_write_register(p, rd, (uint32_t) !operand2);
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
					load_store_address = arm_load_store(p, instr);
					uint32_t rd_value, bit20, bit22, x;
					bit20 = get_bit(res, 20);
					bit22 = get_bit(res, 22);
					rd_value = arm_read_register(p, rd);
					loadStoreByteOrWord(&p, load_store_address, rd_value, bit20, bit22, rd);
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

	void loadStoreByteOrWord(arm_core *p, uint32_t load_store_address, uint32_t rd_value, uint32_t bit20, uint32_t bit22, uint32_t rd) {
		if (!bit20 && !bit22) { // STR
			arm_write_word(*p, load_store_address, rd_value);
			uint32_t x; printf("Just wrote word : %d\n", arm_read_word( *p, load_store_address, &x) );
		}
		else if (!bit20 && bit22) { // STRB
			arm_write_byte(*p, load_store_address, get_bits(rd_value, 7, 0));
			uint8_t x; printf("Just wrote byte : %d\n", arm_read_byte( *p, load_store_address, &x));
		}
		else if (bit20 && !bit22) { // LDR
			uint32_t data, val1, val2; // data = Memory[address,4] Rotate_Right (8 * address[1:0])
			val1 = arm_read_word(*p, load_store_address, &val1);
			val2 = get_bits(load_store_address, 1, 0);
			data = ror(val1, 8 * val2);
			if (rd == 15)  // PC = data AND 0xFFFFFFFE
				arm_write_register(*p, rd, data & 0xFFFFFFFE); // writing to PC
			else 
				arm_write_register(*p, rd, data);
		}
		else if (bit20 && bit22) { // LDRB Rd = Memory[address,1]
			uint8_t x, y; printf("Executing LDRB\n");
			y = arm_read_byte(*p, load_store_address, &x);
			printf("register number (r) = %d, value to load to r = %d\n", rd, y);
			arm_write_register(*p, rd, y);
		}
		else 
			printf("Error during loading or storing a byte or a word to memory\n");

	}

	void update_Flag_Z(arm_core p, int indice){
	  uint32_t flag = arm_read_cpsr(p);
		if (indice==1)
			set_bit(flag, Z);
		else
			clr_bit(flag, Z);
	}

	void update_Flag_N(arm_core p, int indice){
	  	uint32_t flag = arm_read_cpsr(p);
		if (indice==1)
			set_bit(flag, N);
		else
			clr_bit(flag, N);
	}

void update_Flag_C(arm_core p, int indice){
  uint32_t flag = arm_read_cpsr(p);
	if (indice==1)
		set_bit(flag, C);
	else
		clr_bit(flag, C);
}

void update_Flag_V(arm_core p, int indice){
  uint32_t flag = arm_read_cpsr(p);
	if (indice==1)
		set_bit(flag, V);
	else
		clr_bit(flag, V);
}
void update_flag(arm_core p, uint64_t operands_result){
   uint32_t flag = arm_read_cpsr(p);
   if(get_bit(flag,20)==1){ //si S==1 
         if(operands_result==0)
             update_Flag_Z(p,1);
         if(get_bit(operands_result,31)==1)
             update_Flag_N(p,1);
         if(get_bits(operands_result,63,32)!=0b0) { //dans ce cas y'a retenue et overflow
             update_Flag_V(p,1);
         	update_Flag_C(p,1);
     	}
    }
}

	uint32_t shiftedRegisterOperand(arm_core p, uint32_t ins, uint32_t *shifter_carry_out) {
		uint32_t bits4_6, bits7_11, operand2, rm, rs, rs_bits_0_7, rs_bits_0_4, shift_imm, val1, val2, cFlag;
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
