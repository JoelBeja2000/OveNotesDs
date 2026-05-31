# OveNotesDS

> 🌐 **Idioma / Language:** Español | [English](README.md)

<p align="center">
  <img src="https://github.com/JoelBeja2000/OveNotesDs/releases/download/v1.0.0/nds.gif" alt="OveNotesDS Demo" width="400">
</p>

---

> [!IMPORTANT]
> ### 🚀 Enlaces de Descarga Directa (Última Versión)
> Descarga la versión compilada y lista para usar en tus dispositivos:
> 
> * **🎮 Nintendo DS / DSi (ROM Homebrew):** [Descargar OveNotesDs.nds](https://github.com/JoelBeja2000/OveNotesDs/releases/latest/download/OveNotesDs.nds)
> * **📱 Android (Aplicación Móvil):** [Descargar OveNotesDS.apk](https://github.com/JoelBeja2000/OveNotesDs/releases/latest/download/OveNotesDS.apk)
> * **💻 Windows 64 bits (Portable / Sin instalación):** [Descargar OveNotesDS.exe](https://github.com/JoelBeja2000/OveNotesDs/releases/latest/download/OveNotesDS.exe)
> * **💻 Windows 64 bits (Instalador / Corrige error WebView2):** [Descargar OveNotesDS_x64_installer.exe](https://github.com/JoelBeja2000/OveNotesDs/releases/latest/download/OveNotesDS_x64_installer.exe)
> * **💻 Windows 32 bits (Portable / Sin instalación):** [Descargar OveNotesDS_x86.exe](https://github.com/JoelBeja2000/OveNotesDs/releases/latest/download/OveNotesDS_x86.exe)
> * **💻 Windows 32 bits (Instalador / Corrige error WebView2):** [Descargar OveNotesDS_x86_installer.exe](https://github.com/JoelBeja2000/OveNotesDs/releases/latest/download/OveNotesDS_x86_installer.exe)
> * **🍎 macOS (Aplicación de Escritorio):** [Descargar OveNotesDS.dmg](https://github.com/JoelBeja2000/OveNotesDs/releases/latest/download/OveNotesDS.dmg)

Tienes que tener tu DS conectada a una red wifi y descargarte el archivo .nds, la APK o el .exe. Una vez que estés en la vista para crear notas, haces clic en el botón de menú. Ahí encontrarás un formulario para introducir el código de la DS que te aparece en el PC o en el móvil. También tienes que seleccionar el nombre de tu wifi en ese mismo formulario.

---

Este documento describe la estructura del proyecto, los sistemas principales y el flujo de ejecución de **OveNotesDS**, una aplicación homebrew para Nintendo DS desarrollada con el SDK **[BlocksDS](https://github.com/blocksds)**. Su propósito principal es permitir al usuario dibujar en la pantalla táctil y subir el resultado como imágenes PNG a un servidor mediante peticiones HTTP POST (`/api/nueva-nota`).

---

## 🛠️ Entorno de Desarrollo y Herramientas

*   **SDK**: [BlocksDS](https://github.com/blocksds) (Core).
*   **Lenguaje**: C (ARM9).
*   **Librerías principales**:
    *   `libnds9`: Maneja el hardware de Nintendo DS (pantallas, VRAM, botones y entrada táctil).
    *   `libdswifi9` (`dswifi`): Inicializa y maneja conexiones Wi-Fi.
    *   `lodepng`: Codificador PNG en C que convierte el lienzo en datos PNG comprimidos antes de la subida.

---

## 📂 Estructura del Código Fuente (`/source`)

El código está organizado en módulos independientes para lógica del juego, red, UI y renderizado.

### 1. `main.c` (Punto de Entrada)
*   **Función**: Inicializa excepciones, configura pantallas y monta la tarjeta SD con `fatInitDefault()`.
*   **Flujo**:
    1. Llama a `gameInit()` para configurar VRAM y modos de vídeo.
    2. Si la SD se monta correctamente, inicializa el log con `logInit()` y carga la configuración con `netLoadConfig()`.
    3. Entra en el bucle principal ejecutando `inputScan()`, `gameUpdate()` y esperando `swiWaitForVBlank()` a 60 FPS.

### 2. `game.c` y `game.h` (Máquina de Estados)
*   **Estados de `GameState`**:
    *   `STATE_DRAW`: Pantalla de dibujo principal.
    *   `STATE_WIZARD`: Formulario de configuración de red con teclado virtual.
    *   `STATE_UPLOAD`: Codifica el lienzo a PNG y lo envía por HTTP POST.
*   **`gameUpdate()`**: Procesa la entrada táctil y controla el comportamiento según el estado.
*   **Control táctil**: Usa la lógica de liberación para evitar coordenadas iniciales `0,0` ruidosas.

### 3. `net.c` y `net.h` (Red)
*   **DS vs DSi**: Usa `Wifi_InitDefault(WIFI_ATTEMPT_DSI_MODE)` y cae al modo compatible si es necesario.
*   **`enviarNotaHTTP()`**:
    *   Crea un socket TCP.
    *   Conecta al servidor remoto configurado.
    *   Construye cabeceras POST estándar.
    *   Envía los datos PNG usando sockets no bloqueantes y `select()` para evitar bloqueos.
*   **Persistencia**:
    *   `netLoadConfig()`: Lee `ovenotes_config.txt` desde SD.
    *   `netSaveConfig()`: Guarda IP, puerto y SSID en la SD.

### 4. `ui.c` y `ui.h` (Interfaz)
*   **`uiDrawToolbar()`**: Renderiza la barra de herramientas con pinceles, borrador, CONFIG y PUBLICAR.
*   **`uiDrawFormUI()`**: Dibuja el panel de configuración de red y el teclado en pantalla.

### 5. `render.c` y `render.h` (Renderizado)
*   **Motor**: Dibuja primitivas en el búfer del lienzo.
*   **Funciones**: Dibujo de píxeles, líneas interpoladas y previsualización en la pantalla superior.

### 6. `input.c` y `input.h` (Entrada)
*   Encapsula la lectura de botones y la entrada táctil en una abstracción simple.

### 7. `log.c` y `log.h` (Logs)
*   Sustituye `printf` por `logPrintf`.
*   Escribe la traza de depuración en `sd:/debug_log.txt` y fuerza la escritura inmediata.

---

## Capturas de pantalla

### Inicio de la app
![Inicio de la app](https://github.com/JoelBeja2000/OveNotesDs/releases/download/v1.0.0/InicicioDeLaApp.jpg)

### Nota creada
![Nota creada](https://github.com/JoelBeja2000/OveNotesDs/releases/download/v1.0.0/notaCreada.jpg)

### Pantalla de conexión
![Pantalla de conexión](https://github.com/JoelBeja2000/OveNotesDs/releases/download/v1.0.0/Conexion.jpg)

### Cambio de tema
![Cambio de tema](https://github.com/JoelBeja2000/OveNotesDs/releases/download/v1.0.0/FotoCambioDeTema.jpg)

### Cambio de idioma
![Cambio de idioma](https://github.com/JoelBeja2000/OveNotesDs/releases/download/v1.0.0/Cambiodioma.jpg)

### Captura extra
![Captura extra](https://github.com/JoelBeja2000/OveNotesDs/releases/download/v1.0.0/b9f0dd38-37db-4ba2-9692-18a966a032e4.jpg)

---

## 💬 FAQ (Preguntas Frecuentes)

### 1. ¿Funciona en Nintendo 3DS / 2DS?
**¡Sí!** Puedes iniciar la ROM `.nds` en cualquier 3DS o 2DS (utilizando cargadores de homebrew o accesos directos/forwarders compatibles con `nds-bootstrap`).
*   **Soporte de Wi-Fi WPA2 en 3DS:** Dado que la ROM está compilada con soporte extendido para DSi, si la ejecutas en **Modo DSi**, podrá conectarse directamente a las redes **Wi-Fi WPA2 modernas** configuradas en los ajustes de tu consola.
*   Si la ejecutas en **Modo DS** (por ejemplo, desde un cartucho flashcart clásico), estará limitada a las conexiones de DS clásica (cifrado WEP o punto de acceso abierto).

### 2. ¿Por qué falla la conexión Wi-Fi en mi DS clásica o DS Lite?
El hardware de la DS original y la DS Lite solo admite redes **WEP** o **abiertas (sin contraseña)**. La mayoría de los routers domésticos modernos usan WPA2/WPA3. Para conectar una DS clásica, necesitarás crear un punto de acceso móvil abierto (sin contraseña) de forma temporal en tu móvil.

### 3. ¿Es este proyecto "vibe coded" (asistido por Inteligencia Artificial)?
**¡Sí!** El prototipo y el código se desarrollaron utilizando herramientas de IA para experimentar rápidamente con el desarrollo homebrew en Nintendo DS y crear un flujo de conexión multiplataforma funcional. Es de código abierto, y **estamos encantados de recibir optimizaciones manuales, limpiezas de código o refactorizaciones** por parte de la comunidad homebrew. ¡Siéntete libre de abrir un PR!

### 4. ¿Se puede usar para mensajería instantánea o chat?
Por ahora solo permite dibujar y subir notas como PNG a tu servidor. No obstante, dado que la capa de red usa sockets TCP, expandir el proyecto para admitir un chat bidireccional en tiempo real es totalmente factible y es un excelente candidato para futuras actualizaciones.

### 5. ¿Es seguro conectar mi DS a Internet?
Conectar la DS a un servidor local o privado es seguro. Sin embargo, ten en cuenta que el hardware de la DS utiliza protocolos de red antiguos sin cifrado moderno (HTTP sin SSL/TLS). Evita transmitir datos personales o confidenciales.

---

## 🙏 Agradecimientos

| Contribución | Usuario | Enlace |
|---|---|---|
| Revisión de la localización francesa: correcciones de traducción, teclado AZERTY, compatibilidad de fuentes | **tockyng** (Izuku Midoriya) | [Reddit u/tockyng](https://www.reddit.com/user/tockyng) |
| Creación y mantenimiento de **BlocksDS**, el SDK que ha hecho posible este proyecto | **@AntonioND** | [GitHub @AntonioND](https://github.com/AntonioND) |

> Agradecimiento especial a **tockyng** (Izuku Midoriya) de Reddit por revisar la localización francesa y aportar las correcciones que han hecho que el soporte del idioma francés sea preciso y completo.
>
> Agradecimiento especial a **@AntonioND** por crear y mantener [BlocksDS](https://github.com/blocksds), sin el cual este proyecto no existiría.
