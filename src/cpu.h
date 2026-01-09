#ifndef CPU_H
#define CPU_H

#include "ALU.h"

void decode(int ir, int *opcode, int *addr_mode, int *operand);
int get_operand(int mode, int value);
int get_addr(int mode, int value);
void check_interruptions();
int mainloop();

#endif