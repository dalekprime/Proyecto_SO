#include "dma.h"

// Bucle principal simulando el circuito del DMA
void* dma_loop() {
    Sector buffer; // Buffer interno del DMA (tamaño 9 bytes)
    while (1) {
        while (!sys.dma_controller.active) {
        }
        pthread_mutex_lock(&sys.bus_mutex);
        // Copiar parámetros a registros internos temporales para la operación
        int track = sys.dma_controller.selected_track;
        int cyl = sys.dma_controller.selected_cylinder;
        int sec = sys.dma_controller.selected_sector;
        int ram_addr = sys.dma_controller.ram_address;
        bool mode = sys.dma_controller.io_mode; // 0=Leer (Disk->Mem), 1=Escribir (Mem->Disk) -- Segun enunciado 0=leer, 1=escribir
        // Liberamos el lock del controlador para permitir que la CPU siga (pero seguimos activos)
        int result_status = 0;
        if (mode == 0) {
            //Leer del Disco al Buffer DMA
            memset(&buffer, 0, sizeof(Sector));
            read_sector(track, cyl, sec, (char*)buffer.data);
            //Escribir del Buffer DMA a RAM
            // Transferimos el sector (9 bytes)
            if (ram_addr  < MEM_SIZE && ram_addr >= 0) {
                memory_write(ram_addr, (int)buffer.data);
            } else {
                result_status = 1;
            }
        } else {
            // Leer de RAM al Buffer DMA
            if ((ram_addr) < MEM_SIZE && (ram_addr) >= 0) {
                *buffer.data = (unsigned char)memory_read(ram_addr);
            } else {
                result_status = 1;
            }
            //Escribir del Buffer DMA al Disco
            write_sector(track, cyl, sec, (char*)buffer.data);
        }
        // 3. FINALIZACIÓN
        sys.dma_controller.active = false;
        sys.dma_controller.status = result_status; // 0=exito, 1=error
        // INTERRUMPIR AL PROCESADOR
        sys.pending_interrupt = INT_IO_END;
        pthread_mutex_unlock(&sys.bus_mutex);
    }
    return NULL;
}
