<p align="center">
  <img src="https://github.com/JoelBeja2000/OveNotesDs/releases/download/v1.0.0/nds.gif" alt="OveNotesDS Demo" width="480">
</p>

<h1 align="center">OveNotesDS</h1>

<p align="center">
  <strong>Draw on your Nintendo DS/DSi and send notes wirelessly to any device.</strong>
</p>

<p align="center">
  <a href="https://github.com/JoelBeja2000/OveNotesDs/releases/latest"><img src="https://img.shields.io/github/v/release/JoelBeja2000/OveNotesDs?style=flat-square&label=Latest%20Release&color=ffd100" alt="Latest Release"></a>
  <a href="LICENSE"><img src="https://img.shields.io/github/license/JoelBeja2000/OveNotesDs?style=flat-square&color=ffd100" alt="License"></a>
  <a href="https://github.com/JoelBeja2000/OveNotesDs/stargazers"><img src="https://img.shields.io/github/stars/JoelBeja2000/OveNotesDs?style=flat-square&color=ffd100" alt="Stars"></a>
</p>

<p align="center">
  🌐 English | <a href="README.es.md">Español</a>
</p>

---

## What is OveNotesDS?

OveNotesDS is a **Nintendo DS / DSi homebrew app** that turns your console into a wireless drawing pad. Draw directly on the touchscreen, then send your note as a PNG image to a companion app running on your **PC, Mac, or Android phone** — all over your local Wi-Fi network.

### ✨ Features

- 🖊️ **Freehand drawing** on the DS touchscreen with multiple brush sizes
- 🧹 **Eraser tool** and **bucket fill**
- 🎨 **Color palette** with custom colors
- 🖼️ **Layer system** — work with multiple independent drawing layers
- 📐 **Perspective grid** with vanishing points for technical drawing
- 🌐 **Wireless upload** — send notes as PNG to any device on your network
- 🗂️ **Gallery view** — browse, preview and download your notes from the companion app
- 🌍 **3 languages** — Spanish, English, French (with AZERTY keyboard)
- 🎨 **Multiple themes** — choose your color scheme

---

## 📥 Downloads

> [!IMPORTANT]
> **New here? Just grab the files below — no technical knowledge needed.**

| Platform | Download |
|---|---|
| 🎮 Nintendo DS / DSi (ROM) | [**OveNotesDs.nds**](https://github.com/JoelBeja2000/OveNotesDs/releases/latest/download/OveNotesDs.nds) |
| 📱 Android | [**OveNotesDS.apk**](https://github.com/JoelBeja2000/OveNotesDs/releases/latest/download/OveNotesDS.apk) |
| 💻 Windows 64-bit (Portable) | [**OveNotesDS.exe**](https://github.com/JoelBeja2000/OveNotesDs/releases/latest/download/OveNotesDS.exe) |
| 💻 Windows 64-bit (Installer) | [**OveNotesDS_x64_installer.exe**](https://github.com/JoelBeja2000/OveNotesDs/releases/latest/download/OveNotesDS_x64_installer.exe) |
| 💻 Windows 32-bit (Portable) | [**OveNotesDS_x86.exe**](https://github.com/JoelBeja2000/OveNotesDs/releases/latest/download/OveNotesDS_x86.exe) |
| 💻 Windows 32-bit (Installer) | [**OveNotesDS_x86_installer.exe**](https://github.com/JoelBeja2000/OveNotesDs/releases/latest/download/OveNotesDS_x86_installer.exe) |
| 🍎 macOS | [**OveNotesDS.dmg**](https://github.com/JoelBeja2000/OveNotesDs/releases/latest/download/OveNotesDS.dmg) |

> 💡 Use the **Installer** version on Windows if you get a WebView2 error when launching the app.

---

## 🚀 Quick Start (5 minutes)

> **Requirements:** DS/DSi connected to Wi-Fi · PC/Mac/Android on the **same** network

1. **Install the companion app** on your PC, Mac or Android (see Downloads above).
2. **Copy `OveNotesDs.nds`** to your DS flashcard SD card.
3. **Launch the companion app** — it will show a **6-character connection code**.
4. **Launch `OveNotesDs.nds`** on your DS, go to the drawing view, tap **MENU**.
5. **Enter the connection code** shown in the companion app and select your Wi-Fi network.
6. **Draw something**, then tap **SEND** — your note appears instantly in the companion app!

---

## 📸 Screenshots

<table>
  <tr>
    <td align="center"><img src="https://github.com/JoelBeja2000/OveNotesDs/releases/download/v1.0.0/InicicioDeLaApp.jpg" width="220" alt="App launch"><br><sub>App launch</sub></td>
    <td align="center"><img src="https://github.com/JoelBeja2000/OveNotesDs/releases/download/v1.0.0/notaCreada.jpg" width="220" alt="Note created"><br><sub>Note created</sub></td>
    <td align="center"><img src="https://github.com/JoelBeja2000/OveNotesDs/releases/download/v1.0.0/Conexion.jpg" width="220" alt="Connection"><br><sub>Connection screen</sub></td>
  </tr>
  <tr>
    <td align="center"><img src="https://github.com/JoelBeja2000/OveNotesDs/releases/download/v1.0.0/FotoCambioDeTema.jpg" width="220" alt="Themes"><br><sub>Theme selector</sub></td>
    <td align="center"><img src="https://github.com/JoelBeja2000/OveNotesDs/releases/download/v1.0.0/Cambiodioma.jpg" width="220" alt="Languages"><br><sub>Language switcher</sub></td>
    <td align="center"><img src="https://github.com/JoelBeja2000/OveNotesDs/releases/download/v1.0.0/b9f0dd38-37db-4ba2-9692-18a966a032e4.jpg" width="220" alt="Extra"><br><sub>Drawing view</sub></td>
  </tr>
</table>

---

## 💬 FAQ

<details>
<summary><strong>Does it work on 3DS / 2DS?</strong></summary>

**Yes!** Run the `.nds` ROM via `nds-bootstrap` or any homebrew launcher.

- **DSi Mode** → full WPA2 Wi-Fi support (recommended)
- **DS Mode** (flashcart) → WEP or open networks only
</details>

<details>
<summary><strong>Wi-Fi not connecting on original DS / DS Lite?</strong></summary>

The original DS hardware only supports **WEP** or **open (no password)** networks. Create a temporary open hotspot on your phone and connect the DS to it.
</details>

<details>
<summary><strong>Is it safe to use?</strong></summary>

Safe for local/private use. The DS uses plain HTTP (no SSL), so avoid sending sensitive data over public networks.
</details>

<details>
<summary><strong>Can it be used for chat / messaging?</strong></summary>

Not currently — it only sends PNG drawings. But since it uses TCP sockets, adding real-time chat is possible and a great PR idea!
</details>

<details>
<summary><strong>Is this AI-assisted / vibe coded?</strong></summary>

Yes! Built with AI tools to explore DS homebrew development. It's fully open-source — manual improvements, refactors and PRs are very welcome.
</details>

---

## 🛠️ For Developers

### Tech Stack

| Component | Technology |
|---|---|
| DS Firmware | C (ARM9), [BlocksDS](https://github.com/blocksds) SDK |
| Networking | `libdswifi9`, raw TCP sockets |
| PNG encoding | `lodepng` (embedded) |
| Companion App | [Tauri](https://tauri.app/) (Rust + Web) |
| Server | Node.js (`server.js`) |
| Android | Tauri Android target |

### Building from Source

#### DS ROM

1. Install [BlocksDS](https://github.com/blocksds/sdk) and [Wonderful Toolchain](https://wonderful.asie.pl/).
2. Clone this repo.
3. Run:
```bash
make
```
Output: `OveNotesDs.nds`

#### Companion App (Windows / macOS / Android)

```bash
npm install
npm run tauri build          # Windows / macOS
npm run tauri android build  # Android APK
```

### Project Structure

```
OveNotesDs/
├── source/
│   ├── main.c              # Entry point, main loop
│   ├── systems/
│   │   ├── game.c/.h       # State machine (DRAW / WIZARD / UPLOAD)
│   │   ├── net.c/.h        # Wi-Fi init, TCP socket, HTTP POST
│   │   ├── render.c/.h     # Pixel drawing, line interpolation
│   │   ├── input.c/.h      # Button & touchscreen abstraction
│   │   └── log.c/.h        # SD card debug logging
│   └── ui/
│       ├── ui.c/.h         # Coordinator / view router
│       ├── forms/          # UI Input forms (form_wifi.c, form_rename.c)
│       ├── modals/         # UI Modals (colors, tools, backgrounds, brush, confirm, language...)
│       ├── views/          # Screen views (view_canvas.c, view_menu.c, view_gallery.c, view_wizard.c)
│       ├── widgets/        # Low-level widgets (keyboard.c, sidebar.c, toolbar.c, widget_buttons.c)
│       ├── language_en.c   # English localization strings
│       ├── language_es.c   # Spanish localization strings
│       └── language_fr.c   # French localization strings
├── dist/index.html         # Web companion UI (served by server.js)
├── server.js               # Node.js pairing & gallery server
├── src-tauri/              # Tauri app shell
└── Makefile
```

### Contributing

Pull requests are welcome! Some ideas:
- 🔤 Add a new language translation
- 🎨 Add a new theme
- 🐛 Bug fixes
- ⚡ Performance improvements
- 🔐 HTTPS/TLS support for the server

Open an issue first for large changes so we can discuss the approach.

---

## 🙏 Credits

| Contribution | User | Link |
|---|---|---|
| Created and maintains **BlocksDS**, the SDK that made this project possible | **@AntonioND** | [GitHub @AntonioND](https://github.com/AntonioND) |
| French localization review: translations, AZERTY keyboard, font compatibility | **tockyng** (Izuku Midoriya) | [Reddit u/tockyng](https://www.reddit.com/user/tockyng) |

---

## 📄 License

[MIT](LICENSE) © JoelBeja2000
