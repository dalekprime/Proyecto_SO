#include "dma.h"

// Inicialización del DMA
void init_dma(void) {

    pthread_mutex_init(&sys.dma_controller.mutex, NULL);
    pthread_cond_init(&sys.dma_controller.cond, NULL);
    sys.dma_controller.active = false;
    sys.dma_controller.status = 0; // 0 = Exito por defecto

    // Crear el hilo del DMA (hardware device)
    pthread_t dma_thread;
    pthread_create(&dma_thread, NULL, dma_loop, NULL);
    pthread_detach(dma_thread);
}

// Bucle principal simulando el circuito del DMA
void* dma_loop(void* arg) {
    Sector buffer; // Buffer interno del DMA (tamaño 9 bytes)

    while (1) {
        // Esperar señal sdmaon
        pthread_mutex_lock(&sys.dma_controller.mutex);
        while (!sys.dma_controller.active) {
            pthread_cond_wait(&sys.dma_controller.cond, &sys.dma_controller.mutex);
        }

        // Copiar parámetros a registros internos temporales para la operación
        int track = sys.dma_controller.selected_track;
        int cyl = sys.dma_controller.selected_cylinder;
        int sec = sys.dma_controller.selected_sector;
        int ram_addr = sys.dma_controller.ram_address;
        bool mode = sys.dma_controller.io_mode; // 0=Leer (Disk->Mem), 1=Escribir (Mem->Disk) -- Segun enunciado 0=leer, 1=escribir

        // Liberamos el lock del controlador para permitir que la CPU siga (pero seguimos activos)
        pthread_mutex_unlock(&sys.dma_controller.mutex);

        int result_status = 0;

        if (mode == 0) {


            //Leer del Disco al Buffer DMA
            memset(&buffer, 0, sizeof(Sector));
            read_sector(track, cyl, sec, (char*)buffer.data);

            //Escribir del Buffer DMA a RAM
            // Transferimos el sector (9 bytes)
            for (int i = 0; i < TAM_SECTOR; i++) {
                // ARBITRAJE: Pedir Bus
                pthread_mutex_lock(&sys.bus_mutex);

                // Chequeo de límites físicos
                if ((ram_addr + i) < MEM_SIZE && (ram_addr + i) >= 0) {
                   sys.ram[ram_addr + i] = (int)buffer.data[i];
                } else {
                   result_status = 1;
                }

                // Liberar bus inmediatamente
                pthread_mutex_unlock(&sys.bus_mutex);
            }

        } else {

            // Leer de RAM al Buffer DMA
            for (int i = 0; i < TAM_SECTOR; i++) {
                // Pedir Bus
                pthread_mutex_lock(&sys.bus_mutex);

                if ((ram_addr + i) < MEM_SIZE && (ram_addr + i) >= 0) {
                    buffer.data[i] = (unsigned char)sys.ram[ram_addr + i];
                } else {
                    buffer.data[i] = 0;
                    result_status = 1;
                }

                // Liberar bus
                pthread_mutex_unlock(&sys.bus_mutex);
            }

            //Escribir del Buffer DMA al Disco
            write_sector(track, cyl, sec, (char*)buffer.data);
        }

        // 3. FINALIZACIÓN
        pthread_mutex_lock(&sys.dma_controller.mutex);

        sys.dma_controller.active = false;
        sys.dma_controller.status = result_status; // 0=exito, 1=error

        // INTERRUMPIR AL PROCESADOR
        sys.pending_interrupt = INT_IO_END;

        pthread_mutex_unlock(&sys.dma_controller.mutex);
    }
    
    return NULL;
}
