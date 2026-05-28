@echo off

echo ====================================================
echo Compilando OveNotesDS con BlocksDS...
echo ====================================================
echo.

C:\msys64\usr\bin\bash.exe -l -c "source /opt/wonderful/bin/wf-env && cd \"$(cygpath '%~dp0')\" && make clean && make"

if %ERRORLEVEL% EQU 0 (
    echo.
    if exist F:\ (
        copy OveNotesDs.nds F:\OveNotesDs.nds >nul
        echo [SD CARD] ROM copiada automaticamente a F:\OveNotesDs.nds
        if exist F:\roms\nds\ (
            copy OveNotesDs.nds F:\roms\nds\OveNotesDs.nds >nul
            echo [SD CARD] ROM copiada automaticamente a F:\roms\nds\OveNotesDs.nds
        )
    )
    echo.
    echo ====================================================
    echo [OK] ROM compilada con exito!
    echo Se ha generado/actualizado: OveNotesDs.nds
    echo ====================================================
) else (
    echo.
    echo ====================================================
    echo [ERROR] Hubo un fallo en la compilacion.
    echo ====================================================
)

echo.
