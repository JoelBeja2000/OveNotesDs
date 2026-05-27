@echo off
title Servidor OveNotes DS
chcp 65001 > nul
echo ===================================================
echo 🚀 Iniciando Servidor OveNotes DS...
echo ===================================================
echo.

:: Verificar si Node.js está instalado
where node >nul 2>nul
if %errorlevel% neq 0 (
    echo ❌ [ERROR] Node.js no está instalado en este sistema.
    echo Por favor, descarga e instala Node.js desde https://nodejs.org/
    echo.
    pause
    exit /b
)

:: Instalar dependencias si no existe node_modules
if not exist node_modules (
    echo 📦 [INFO] Instalando dependencias de Node.js...
    call npm install
    echo.
)

:: Abrir el navegador en el puerto 3000
echo 🌐 [INFO] Abriendo la consola web en tu navegador...
start http://localhost:3000

:: Ejecutar el servidor
echo 💻 [INFO] Servidor en ejecución. Presiona Ctrl+C para detener.
echo.
node server.js

pause
