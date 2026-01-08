#include "system.h"

//Decodificador de Instrucciones
void decode(int ir, int *opcode, int* addr_mode, int *operand){

};
//Calcula Operando
int get_operand(int mode, int value){
    switch (mode){
        case 0:
            
        break;
        case 1:
            
        break;
        case 2:
            
        break;
        default:
        break;
    }
}
//Manejador de Interrupciones
void check_interruptions(){
    if(sys.pending_interrupt != INT_NONE && sys.cpu_registers.PSW.interruptions_enabled){

    };
}
//Main Loop del CPU
int mainloop(){
    while(1){
        //Fetch Phase

        //Decode Phase

        //Execute Phase

    };
};

