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
#include "arm_branch_other.h"
#include "arm_constants.h"
#include "util.h"
#include <debug.h>
#include <stdlib.h>
#include "arm_instruction.h"


uint32_t Extend_30(uint32_t x, uint32_t ins){
      x = get_bits(ins, 23, 0);
      if( get_bit(ins, 23) == 0)
         return set_bits(x, 30, 24,0b000000);
      else
         return set_bits(x, 30, 24,0b111111);
}


int arm_branch(arm_core p, uint32_t ins) { 
       uint32_t x;
  
       if(get_bit(ins,24)==0){  /*c'est un branchement B
           x est l'extension ds 24 bits d'offset, décalé de 2 bits à g*/ 
           x = Extend_30(x, ins) << 2;
           /*pc<-pc+x*/
           arm_write_register(p,15, arm_read_register(p,15)+x);
                    
       }else{
        /*c'est un BL
         l'instruct courante est mise dans LR, puis branchmt!*/
         uint32_t adr = arm_read_register(p,15);
         arm_write_register(p, 14, adr-0x04);  
         x = Extend_30(x, ins) << 2;
         arm_write_register(p,15, arm_read_register(p,15)+x);
        
      } 
    
     return 0;
    //return UNDEFINED_INSTRUCTION;
}

int arm_coprocessor_others_swi(arm_core p, uint32_t ins) {
    if (get_bit(ins, 24)) {
        /* Here we implement the end of the simulation as swi 0x123456 */
        if ((ins & 0xFFFFFF) == 0x123456)
            exit(0);
        return SOFTWARE_INTERRUPT;
    }  
    return UNDEFINED_INSTRUCTION;
}

int arm_miscellaneous(arm_core p, uint32_t ins) {
    return UNDEFINED_INSTRUCTION;
}
