#include "loader.h"

// Función genérica para registrar cualquier evento
void write_in_log(const char *event) {
    FILE *archive = fopen("resume.log", "a");
    if (archive == NULL) return;
    fprintf(archive, "Instruccion Ejecutada: %s\n", event);
    fclose(archive);
};

