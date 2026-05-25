#include <nds.h>
#include <fat.h>
#include <stdio.h>
#include "game.h"
#include "input.h"
#include "log.h"
#include "net.h"

int main(void) {
    // Registrar el manejador de excepciones de hardware por defecto
    defaultExceptionHandler();
    
    // Inicializar el motor de juego, pantallas y modos de video
    gameInit();

    // Inicializar el sistema de archivos de la tarjeta SD
    printf("[SYS] Inicializando FAT (SD Card)...\n");
    if (fatInitDefault()) {
        logInit();
        printf("[SYS] Tarjeta SD montada correctamente y log activo.\n");
        netLoadConfig();
    } else {
        printf("[FATAL ERROR] No se pudo inicializar la SD (fatInitDefault fallo).\n");
    }
    
    // Bucle de interaccion principal coordinado por la maquina de estados
    while (1) {
        // Escanear entrada del hardware (botones y lapiz tactil)
        inputScan();
        
        // Actualizar el estado activo del juego (dibujo, wizard, subida)
        gameUpdate();
        
        // Esperar sincronizacion vertical a 60 FPS
        swiWaitForVBlank();
    }
    
    return 0;
}
