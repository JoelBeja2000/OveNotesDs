$env:DEVKITPRO = '/d/Proyectos/OveNotesDs/devKitPro'
$env:DEVKITARM = '/d/Proyectos/OveNotesDs/devKitPro/devkitARM'
$env:PATH = 'D:\Proyectos\OveNotesDs\devKitPro\msys2\usr\bin;D:\Proyectos\OveNotesDs\devKitPro\devkitARM\bin;D:\Proyectos\OveNotesDs\devKitPro\tools\bin;' + $env:PATH

Write-Host "Running make clean..."
make clean
Write-Host "Running make..."
make

# Check if make succeeded
if (Test-Path 'OveNotesDs.nds') {
    Write-Host "Patching ROM header..."
    $bytes = [System.IO.File]::ReadAllBytes('OveNotesDs.nds')
    $bytes[0x10] = 48 # '0'
    $bytes[0x11] = 48 # '0'
    $bytes[0x12] = 0  # DS Mode (NTR)
    [System.IO.File]::WriteAllBytes('OveNotesDs.nds', $bytes)
    
    Write-Host "Running ndstool to fix Header CRC..."
    & "D:\Proyectos\OveNotesDs\devKitPro\tools\bin\ndstool.exe" -f OveNotesDs.nds
    
    Write-Host "Compilation and patching complete!"
} else {
    Write-Error "Compilation failed! OveNotesDs.nds was not created."
}
