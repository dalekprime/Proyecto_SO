#include "main.h"

SYSTEM_STATE sys;

void init(){
    //Iniciar Registros
    sys.cpu_registers.AC = 0;
    sys.cpu_registers.IR = 0;
    sys.cpu_registers.MAR = 0;
    sys.cpu_registers.MDR = 0;
    sys.cpu_registers.RB = 300;
    sys.cpu_registers.RL = 300;
    sys.cpu_registers.RX = 300;
    sys.cpu_registers.SP = 300;
    //Iniciar Estado
    sys.cpu_registers.PSW.condition_code = 0;
    sys.cpu_registers.PSW.operation_mode = 0;
    sys.cpu_registers.PSW.interruptions_enabled = 1;
    sys.cpu_registers.PSW.pc = 0;
    //Inciar Parametros    
    sys.debug_mode_enabled = 1;
    sys.time = 0;
    sys.pending_interrupt = INT_NONE;
    //Iniciar Semaforos
    pthread_mutex_init(&sys.bus_mutex, NULL);
    pthread_mutex_init(&sys.log_mutex, NULL);
    //Inciar DMA
    sys.dma_controller.selected_cylinder = 0;
    sys.dma_controller.selected_sector = 0;
    sys.dma_controller.selected_track = 0;
    sys.dma_controller.ram_address = 0;
    sys.dma_controller.status= 0;
};

int main(){

    init();

    int prog_size = load_program("data/program.asm", sys.cpu_registers.RB);
    pthread_t cpu;
    if (prog_size > 0) {
        int code_end = sys.cpu_registers.RB + prog_size;
        sys.cpu_registers.SP = code_end;
        sys.cpu_registers.RX = sys.cpu_registers.SP;
        sys.cpu_registers.RL = code_end + MAX_STACK_SIZE;
        pthread_create(&cpu, NULL, (void*)mainloop, (void*)&prog_size);
        pthread_join(cpu, NULL);
    }

    return 0;
}