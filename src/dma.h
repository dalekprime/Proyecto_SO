#ifndef DMA_H
#define DMA_H

#include "system.h"
#include "disk.h"

// Inicializa el hilo del DMA y sus sincronizadores
void init_dma(void);

// Bucle principal del DMA
void* dma_loop(void* arg);

#endif
