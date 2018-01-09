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
#include <stdlib.h>
#include "memory.h"
#include "util.h"

struct memory_data {
  int is_big_endian;
	uint8_t *address;
	size_t size;
};

memory memory_create(size_t size, int is_big_endian) {
  memory mem = malloc(sizeof(struct memory_data));
  mem->address=malloc(size);
  if (mem->address == NULL)
      mem->size = 0;
  else
      mem->size = size;
  mem->is_big_endian = is_big_endian;

  return mem;

}

size_t memory_get_size(memory mem) {
    size_t  value;
    value= mem->size;
    return value;
}

void memory_destroy(memory mem) {
  free(mem->address);
  free(mem);
}

int memory_read_byte(memory mem, uint32_t address, uint8_t *value) {
  if (address <= mem->size) {
    *value = mem->address[address];
    return *value;
  }
  else
    return -1;
}

int memory_read_half(memory mem, uint32_t address, uint16_t *value) {
  if (address <= mem->size) {
    if (mem->is_big_endian == 0){
      uint16_t a = mem->address[address] ;
      uint16_t b = mem->address[address+1] <<8;
      *value= a | b ;
      return *value;
    }
    else
    {
      uint16_t a = mem->address[address]<<8 ;
      uint16_t b = mem->address[address+1] ;
      *value= a | b ;
      return *value;
    }

  }
  else
    return -1;

}

int memory_read_word(memory mem, uint32_t address, uint32_t *value) {
  if (address <= mem->size) {
    if (mem->is_big_endian == 0){
      uint32_t a = mem->address[address] ;
      uint32_t b = mem->address[address+1] <<8;
      uint32_t c = mem->address[address+2] <<16;
      uint32_t d = mem->address[address+3] <<24;


      *value= a | b | c | d;
      return *value;
    }
    else
    {
      uint32_t a = mem->address[address]<<24 ;
      uint32_t b = mem->address[address+1] << 16;
      uint32_t c = mem->address[address+2] <<8;
      uint32_t d = mem->address[address+3] ;


      *value= a | b | c | d  ;
      return *value;
    }
  }
  else
    {
      return -1;
    }
}

int memory_write_byte(memory mem, uint32_t address, uint8_t value) {
    mem->address[address] = value;
// >>>>>>> 5ce39e2a1df85e6042dd7d84a2254e73a4df25fd
    return 0;
}

int memory_write_half(memory mem, uint32_t address, uint16_t value) {

//  <<<<<<< HEAD
   
    //return -1;
// =======

    if (mem->is_big_endian == 1){
      uint8_t a = value;
      uint8_t b = value >> 8 ;
      memory_write_byte(mem , address , b);
      memory_write_byte( mem ,  address + 1, a);

      
      return 0;
    }
    else {
      uint8_t a = value;
      uint8_t b = value >> 8 ;
      memory_write_byte( mem , address , a);
      memory_write_byte( mem ,  address+1 , b);


      return 0;

    }

// >>>>>>> 5ce39e2a1df85e6042dd7d84a2254e73a4df25fd
}

int memory_write_word(memory mem, uint32_t address, uint32_t value) {

    if (mem->is_big_endian == 1){
      uint32_t a = value;
      uint32_t b = value >> 8 ;
      uint32_t c = value >> 16 ;
      uint32_t d = value >> 24 ;
      mem->address[address] = d  ;
      mem->address[address + 1] = c ;
      mem->address[address + 2] = b ;
      mem->address[address + 3] = a ;
      return 0;
    }
    else {
      uint32_t a = value;
      uint32_t b = value >> 8 ;
      uint32_t c = value >> 16 ;
      uint32_t d = value >> 24 ;
      mem->address[address] = a  ;
      mem->address[address + 1] = b ;
      mem->address[address + 2] = c ;
      mem->address[address + 3] = d ;
      return 0;
    }

}

