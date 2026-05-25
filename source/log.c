#define LOG_INTERNAL
#include "log.h"
#include <stdio.h>
#include <fat.h>
#include <nds.h>
#include <unistd.h>

static FILE* f_log = NULL;

void logInit(void) {
    if (f_log != NULL) return;

    // Intentar abrir el archivo de log en la SD interna de DSi (sd:/)
    f_log = fopen("sd:/debug_log.txt", "w");
    if (f_log == NULL) {
        // Intentar en la ranura de cartucho Slot-1 (fat:/)
        f_log = fopen("fat:/debug_log.txt", "w");
    }
    if (f_log == NULL) {
        // Caída a ruta relativa
        f_log = fopen("debug_log.txt", "w");
    }

    if (f_log != NULL) {
        setvbuf(f_log, NULL, _IONBF, 0); // Guardar al instante en la SD
        fprintf(f_log, "[LOG] Sistema de logs iniciado con exito.\n");
        fprintf(f_log, "[LOG] Consola: %s\n", isDSiMode() ? "DSi Mode" : "DS Mode");
        fflush(f_log);
        fsync(fileno(f_log));
    }
}

int logPrintf(const char* format, ...) {
    va_list args;
    int ret = 0;

    // 1. Imprimir a la consola en pantalla (stdout original)
    va_start(args, format);
    ret = vprintf(format, args);
    va_end(args);

    // 2. Imprimir en el archivo debug_log.txt en la SD
    if (f_log != NULL) {
        va_start(args, format);
        vfprintf(f_log, format, args);
        fflush(f_log);
        fsync(fileno(f_log)); // Forzar actualizacion de tamaño y datos en el FAT de la SD
        va_end(args);
    }

    return ret;
}

void logClose(void) {
    if (f_log != NULL) {
        fprintf(f_log, "[LOG] Cerrando log.\n");
        fclose(f_log);
        f_log = NULL;
    }
}
