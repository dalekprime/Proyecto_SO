#include "cpu.h"

//Decodificador de Instrucciones
void decode(int ir, int *opcode, int *addr_mode, int *operand){
    *opcode = ir / 1000000;
    *addr_mode = (ir / 100000) % 10;
    *operand = ir % 100000;
};

//Calcula Operando
int get_operand(int mode, int value){
    switch (mode){
        case 0:
            //Valor en Direccion
            return sign_to_int(memory_read(value), 8);
        case 1:
            //Valor Inmediato
            return sign_to_int(value, 5);
        case 2:
            //Valor en Memoria con desplazamiento
            int base = sign_to_int(sys.cpu_registers.AC, 8);
            return sign_to_int(memory_read(base + value), 8);
        default:
            //Codigo invalido
            sys.pending_interrupt = INT_INVALID_INSTR;
            return 0;
    }
};

//Calcula Direcciones
int get_addr(int mode, int value){
    switch (mode){
        case 0:
            //Valor en Direccion
            return value;
        case 1:
            //No puede contener ese mode
            sys.pending_interrupt = INT_INVALID_INSTR;
            return -1;
        case 2:
            //Valor en Memoria con desplazamiento
            return sign_to_int(sys.cpu_registers.AC, 8) + value;
        default:
            //Codigo invalido
            sys.pending_interrupt = INT_INVALID_INSTR;
            return 0;
    }
};

//Manejador de Interrupciones
void check_interruptions(){
    if(sys.pending_interrupt != INT_NONE && 
        sys.cpu_registers.PSW.interruptions_enabled){
            //Salvaguardar Estado
            sys.cpu_registers.PSW.operation_mode = 1;
            sys.cpu_registers.PSW.interruptions_enabled = 0;
            //Manejador de Interrupciones
            sys.pending_interrupt = INT_NONE;
    };
}

//Main Loop del CPU
int mainloop(){
    int internal_timer = 0;
    while(1){
        //Interrumpcion de Reloj
        if(internal_timer >= CLOCK_INTERRUPTION_INTERVAL){
            sys.pending_interrupt = INT_TIMER;
            internal_timer = 0;
        };
        //Fetch Phase
        sys.cpu_registers.MAR = sys.cpu_registers.RB + sys.cpu_registers.PSW.pc;
        sys.cpu_registers.MDR = memory_read(sys.cpu_registers.MAR);
        sys.cpu_registers.IR = sys.cpu_registers.MDR;
        sys.cpu_registers.PSW.pc++;
        //Decode Phase
        int opcode, addr_mode, operand;
        decode(sys.cpu_registers.IR, &opcode, &addr_mode, &operand);
        //Debug
        if (sys.debug_mode_enabled == 1) {
            printf("\nDEBUG > MAR: %d | IR: %d| AC: %d\n", 
                   sys.cpu_registers.MAR, 
                   sys.cpu_registers.IR, 
                   sys.cpu_registers.AC);
            printf("Presione Enter para continuar...");
            getchar();
        }
        //Execute Phase
        int val_ac, val_op, res, eff_addr;
        switch (opcode){
            //Sum
            case 0:
                val_ac = sign_to_int(sys.cpu_registers.AC, 8);
                val_op = get_operand(addr_mode, operand);
                res = ALU(val_ac, val_op, SUM);
                sys.cpu_registers.AC = int_to_sign(res, 8);
            break;
            //res
            case 1:
                val_ac = sign_to_int(sys.cpu_registers.AC, 8);
                val_op = get_operand(addr_mode, operand);
                res = ALU(val_ac, val_op, RES);
                sys.cpu_registers.AC = int_to_sign(res, 8);
            break;
            //mult
            case 2:
                val_ac = sign_to_int(sys.cpu_registers.AC, 8);
                val_op = get_operand(addr_mode, operand);
                res = ALU(val_ac, val_op, MULT);
                sys.cpu_registers.AC = int_to_sign(res, 8);
            break;
            //divi
            case 3:
                val_ac = sign_to_int(sys.cpu_registers.AC, 8);
                val_op = get_operand(addr_mode, operand);
                res = ALU(val_ac, val_op, DIVI);
                sys.cpu_registers.AC = int_to_sign(res, 8);
            break;
            //load
            case 4:
                val_op = get_operand(addr_mode, operand);
                sys.cpu_registers.AC = int_to_sign(val_op, 8);
            break;
            //str
            case 5:
                eff_addr = get_addr(addr_mode, operand);
                memory_write(eff_addr, sys.cpu_registers.AC);
            break;
            //loadrx
            case 6:
                sys.cpu_registers.AC = sys.cpu_registers.RX;
            break;
            //strrx
            case 7:
                sys.cpu_registers.RX = sys.cpu_registers.AC;
            break;
            //comp
            case 8:
                val_ac = sign_to_int(sys.cpu_registers.AC, 8);
                val_op = get_operand(addr_mode, operand);
                ALU(val_ac, val_op, RES);
            break;
            //jmpe
            case 9:
                val_ac = sign_to_int(sys.cpu_registers.AC, 8);
                val_op = sign_to_int(memory_read(sys.cpu_registers.SP), 8);
                if(val_ac == val_op){
                    sys.cpu_registers.PSW.pc = get_addr(addr_mode, operand);
                };
            break;
            //jmpne
            case 10:
                val_ac = sign_to_int(sys.cpu_registers.AC, 8);
                val_op = sign_to_int(memory_read(sys.cpu_registers.SP), 8);
                if(val_ac != val_op){
                    sys.cpu_registers.PSW.pc = get_addr(addr_mode, operand);
                };
            break;
            //jmplt
            case 11:
                val_ac = sign_to_int(sys.cpu_registers.AC, 8);
                val_op = sign_to_int(memory_read(sys.cpu_registers.SP), 8);
                if(val_ac < val_op){
                    sys.cpu_registers.PSW.pc = get_addr(addr_mode, operand);
                };
            break;
            //jmplgt
            case 12:
                val_ac = sign_to_int(sys.cpu_registers.AC, 8);
                val_op = sign_to_int(memory_read(sys.cpu_registers.SP), 8);
                if(val_ac > val_op){
                    sys.cpu_registers.PSW.pc = get_addr(addr_mode, operand);
                };
            break;
            //svc
            case 13:
                sys.pending_interrupt = INT_SYSCALL;
                //Salvaguardar Estado
            break;
            //retrn
            case 14:
                //Asumimos que en SP hay una dir de retorno
                sys.cpu_registers.PSW.pc = memory_read(sys.cpu_registers.SP);
            break;
            //hab
            case 15:
                sys.cpu_registers.PSW.interruptions_enabled = 1;
            break;
            //dhab
            case 16:
                sys.cpu_registers.PSW.interruptions_enabled = 0;
            break;
            //tti
            case 17:
                sys.time = operand;
                internal_timer = 0;
            break;
            //chmod
            case 18:
                if(sys.cpu_registers.PSW.operation_mode == 1){
                    sys.cpu_registers.PSW.operation_mode = 0;
                }else{
                    sys.cpu_registers.PSW.operation_mode = 1;
                };
            break;
            //loadrb
            case 19:
                sys.cpu_registers.AC = sys.cpu_registers.RB;
            break;
            //strrb
            case 20:
                sys.cpu_registers.RB = sys.cpu_registers.AC;
            break;
            //loadrl
            case 21:
                sys.cpu_registers.AC = sys.cpu_registers.RL;
            break;
            //strrl
            case 22:
                sys.cpu_registers.RL = sys.cpu_registers.AC;
            break;
            //loadsp
            case 23:
                sys.cpu_registers.AC = sys.cpu_registers.SP;
            break;
            //strsp
            case 24:
                sys.cpu_registers.SP = sys.cpu_registers.AC;
            break;
            //psh
            case 25:
                sys.cpu_registers.SP++;
                memory_write(sys.cpu_registers.SP, sys.cpu_registers.AC);
            break;
            //pop
            case 26:
                sys.cpu_registers.AC = memory_read(sys.cpu_registers.SP);
                sys.cpu_registers.SP--;
            break;
            //j
            case 27:
                sys.cpu_registers.PSW.pc = get_addr(addr_mode, operand);
            break;
            //sdmap
            case 28:
                val_op = get_operand(addr_mode, operand);
                sys.dma_controller.selected_track = val_op;
            break;
            //sdmac
            case 29:
                val_op = get_operand(addr_mode, operand);
                sys.dma_controller.selected_cylinder = val_op;
            break;
            //sdmas
            case 30:
                val_op = get_operand(addr_mode, operand);
                sys.dma_controller.selected_sector = val_op;
            break;
            //sdmaio
            case 31:
                val_op = get_operand(addr_mode, operand);
                sys.dma_controller.io_mode = val_op;
            break;
            //sdmam
            case 32:
                val_op = get_operand(addr_mode, operand);
                sys.dma_controller.ram_address = val_op;
            break;
            //sdmaon
            case 33:
                //Iniciar DMA
            break;
            //Instruccion Invalida
            default:
                sys.pending_interrupt = INT_INVALID_INSTR;
            break;
        }
        sys.time += 1;
        internal_timer += 1;
        check_interruptions();
    };
};

