# OveNotesDS

> 🌐 **Language / Idioma:** English | [Español](README.es.md)

<p align="center">
  <img src="https://github.com/JoelBeja2000/OveNotesDs/releases/download/v1.0.0/nds.gif" alt="OveNotesDS Demo" width="400">
</p>

---

> [!IMPORTANT]
> ### 🚀 Direct Download Links (Latest Version)
> Download the compiled, ready-to-use version for your devices:
> 
> * **🎮 Nintendo DS / DSi (Homebrew ROM):** [Download OveNotesDs.nds](https://github.com/JoelBeja2000/OveNotesDs/releases/latest/download/OveNotesDs.nds)
> * **📱 Android (Mobile App):** [Download OveNotesDS.apk](https://github.com/JoelBeja2000/OveNotesDs/releases/latest/download/OveNotesDS.apk)
> * **💻 Windows 64-bit (Portable App):** [Download OveNotesDS.exe](https://github.com/JoelBeja2000/OveNotesDs/releases/latest/download/OveNotesDS.exe)
> * **💻 Windows 64-bit (Installer / Fixes WebView2 error):** [Download OveNotesDS_x64_installer.exe](https://github.com/JoelBeja2000/OveNotesDs/releases/latest/download/OveNotesDS_x64_installer.exe)
> * **💻 Windows 32-bit (Portable App):** [Download OveNotesDS_x86.exe](https://github.com/JoelBeja2000/OveNotesDs/releases/latest/download/OveNotesDS_x86.exe)
> * **💻 Windows 32-bit (Installer / Fixes WebView2 error):** [Download OveNotesDS_x86_installer.exe](https://github.com/JoelBeja2000/OveNotesDs/releases/latest/download/OveNotesDS_x86_installer.exe)
> * **🍎 macOS (Desktop App):** [Download OveNotesDS.dmg](https://github.com/JoelBeja2000/OveNotesDs/releases/latest/download/OveNotesDS.dmg)

You need to have your DS connected to a Wi-Fi network and download the .nds file, the APK, or the .exe. Once you are in the note creation view, click the menu button. There, you will find a form to enter the DS code that appears on your PC or mobile device. You must also select your Wi-Fi name in that same form.

---

This document describes the project structure, main systems, and execution flow of **OveNotesDS**, a Nintendo DS homebrew application built using the **[BlocksDS](https://github.com/blocksds)** SDK. The main purpose of the application is to let the user draw on the touchscreen and upload the result as PNG images to a server using HTTP POST requests (`/api/nueva-nota`).

---

## 🛠️ Development Environment and Tools

*   **SDK**: [BlocksDS](https://github.com/blocksds) (Core).
*   **Language**: C (ARM9).
*   **Main libraries**:
    *   `libnds9`: Handles Nintendo DS hardware (screens, VRAM, buttons, touchscreen input).
    *   `libdswifi9` (`dswifi`): Initializes and manages Wi-Fi network connections.
    *   `lodepng`: PNG encoder in C for converting the drawing buffer into compressed PNG data before upload.

---

## 📂 Source Structure (`/source`)

The code is organized in independent modules for game logic, networking, UI and rendering.

### 1. `main.c` (Entry Point)
*   **Function**: Initializes exception handling, sets up the screens, and mounts the SD card with `fatInitDefault()`.
*   **Flow**:
    1. Calls `gameInit()` to configure VRAM and video modes.
    2. If SD mount succeeds, initializes logging with `logInit()` and loads saved configuration with `netLoadConfig()`.
    3. Enters the main loop, running `inputScan()`, `gameUpdate()`, and waiting for vertical sync (`swiWaitForVBlank()`) at 60 FPS.

### 2. `game.c` and `game.h` (Application State Machine)
*   **`GameState` states**:
    *   `STATE_DRAW`: Main drawing screen, showing the canvas and toolbar.
    *   `STATE_WIZARD`: Network setup form for IP, port, and Wi-Fi SSID with an on-screen keyboard.
    *   `STATE_UPLOAD`: Encodes the canvas to PNG in RAM and sends it via HTTP POST over TCP sockets.
*   **`gameUpdate()`**: Processes touchscreen input and dispatches events based on the active state.
*   **Touch handling**: Uses release-based logic to avoid noisy initial ADC coordinates like `0,0` when the stylus first touches the screen.

### 3. `net.c` and `net.h` (Networking and Sockets)
*   **DS vs DSi networking**: Uses `Wifi_InitDefault(WIFI_ATTEMPT_DSI_MODE)` to try DSi mode first. If unavailable, falls back to DS-compatible networks.
*   **`enviarNotaHTTP()`**:
    *   Creates a TCP socket (`AF_INET`, `SOCK_STREAM`).
    *   Connects to the configured remote server.
    *   Builds standard HTTP POST headers (`Content-Type: application/octet-stream`, `Content-Length`).
    *   Sends binary PNG data with non-blocking socket support and `select()` timeouts to avoid freezing the main loop.
*   **Configuration persistence**:
    *   `netLoadConfig()`: Reads `ovenotes_config.txt` from the SD root and parses IP, port, and SSID.
    *   `netSaveConfig()`: Writes those values back to SD for later reuse.

### 4. `ui.c` and `ui.h` (User Interface)
*   **`uiDrawToolbar()`**: Renders the bottom toolbar with brush sizes (1, 3, 5), eraser, CONFIG, and UPLOAD.
*   **`uiDrawFormUI()`**: Draws the network configuration panel and virtual keyboard.

### 5. `render.c` and `render.h` (Drawing and Graphics)
*   **Render engine**: Draws primitives directly to the touchscreen canvas buffer.
*   **Functions**: Implements pixel drawing, interpolated line strokes, and preview buffers for the top screen.

### 6. `input.c` and `input.h` (Hardware Input)
*   Wraps button state and touchscreen ADC reading into a simple input abstraction.

### 7. `log.c` and `log.h` (SD Logging)
*   Replaces `printf` with `logPrintf` when `log.h` is included.
*   Writes debug trace to `sd:/debug_log.txt`, flushing immediately to preserve logs after crashes.

---

## 💬 FAQ

### 1. Does this app work on the Nintendo 3DS / 2DS?
**Yes!** You can run the `.nds` ROM on any 3DS or 2DS system (via custom launchers or homebrew forwarders running `nds-bootstrap`).
*   **WPA2 Wi-Fi Support on 3DS:** Since the ROM is compiled with DSi support, launching the application in **DSi Mode** allows it to connect directly to the modern **WPA2 Wi-Fi networks** configured in your 3DS/2DS console's system settings.
*   If you run it in **DS Mode** (e.g., from a standard DS flashcart), it will be restricted to DS-classic network slots (WEP or open/unencrypted hotspots).

### 2. Why is the Wi-Fi connection failing on my original DS / DS Lite?
The original Nintendo DS and DS Lite hardware only support **WEP security** or **unencrypted (open) networks**. Most modern home routers use WPA2 or WPA3. To connect a classic DS, you will need to set up an open (password-less) mobile hotspot on your smartphone.

### 3. Is this project "vibe coded" (AI-assisted)?
**Yes!** The prototype and codebase were built with the assistance of AI tools to rapidly experiment with Nintendo DS homebrew development and build a cross-platform connection flow. It is completely open-source, and **we welcome any manual optimizations, code cleanups, or refactoring** from the homebrew community! Feel free to open a PR.

### 4. Can this be used for instant messaging or chat?
Currently, the app only supports drawing and uploading notes as PNGs to a server. However, since the network layer uses TCP sockets, expanding it to support bidirectional real-time chat is entirely possible. It is a great candidate feature for future updates.

### 5. Is it safe to connect my DS to the internet?
Connecting a DS to a local or private server is safe. However, keep in mind that the DS hardware uses legacy network protocols (HTTP without SSL/TLS). Avoid sending sensitive or personal data.

---

## Screenshots

### App launch
![App launch](https://github.com/JoelBeja2000/OveNotesDs/releases/download/v1.0.0/InicicioDeLaApp.jpg)

### Note created
![Note created](https://github.com/JoelBeja2000/OveNotesDs/releases/download/v1.0.0/notaCreada.jpg)

### Connection screen
![Connection screen](https://github.com/JoelBeja2000/OveNotesDs/releases/download/v1.0.0/Conexion.jpg)

### Theme switch
![Theme switch](https://github.com/JoelBeja2000/OveNotesDs/releases/download/v1.0.0/FotoCambioDeTema.jpg)

### Language switch
![Language switch](https://github.com/JoelBeja2000/OveNotesDs/releases/download/v1.0.0/Cambiodioma.jpg)

### Extra screenshot
![Additional screenshot](https://github.com/JoelBeja2000/OveNotesDs/releases/download/v1.0.0/b9f0dd38-37db-4ba2-9692-18a966a032e4.jpg)

---

## 🙏 Credits

| Contribution | User | Link |
|---|---|---|
| French localization review: translation corrections, AZERTY keyboard layout, font compatibility fixes | **tockyng** (Izuku Midoriya) | [Reddit u/tockyng](https://www.reddit.com/user/tockyng) |
| Created and maintains **BlocksDS**, the SDK that made this entire project possible | **@AntonioND** | [GitHub @AntonioND](https://github.com/AntonioND) |

> Special thanks to **tockyng** (Izuku Midoriya) from Reddit for reviewing the French localization and providing the corrections that made the French language support accurate and complete.
>
> Special thanks to **@AntonioND** for creating and maintaining [BlocksDS](https://github.com/blocksds), without which this project would not exist.
