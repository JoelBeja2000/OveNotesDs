$devkitproWin = Join-Path $PSScriptRoot "devKitPro"
$devkitproMsys = "/" + ($devkitproWin -replace ':', '' -replace '\\', '/')
$env:DEVKITPRO = $devkitproMsys
$env:DEVKITARM = "$devkitproMsys/devkitARM"
$env:PATH = "$devkitproWin\msys2\usr\bin;$devkitproWin\devkitARM\bin;$devkitproWin\tools\bin;" + $env:PATH

Write-Host "Running make clean..."
make clean
Write-Host "Running make..."
make

# Check if make succeeded
if (Test-Path (Join-Path $PSScriptRoot 'OveNotesDs.nds')) {
    Write-Host "Patching ROM header..."
    $romPath = Join-Path $PSScriptRoot 'OveNotesDs.nds'
    $bytes = [System.IO.File]::ReadAllBytes($romPath)
    $bytes[0x10] = 48 # '0'
    $bytes[0x11] = 48 # '0'
    $bytes[0x12] = 0  # DS Mode (NTR)
    [System.IO.File]::WriteAllBytes($romPath, $bytes)
    
    Write-Host "Running ndstool to fix Header CRC..."
    & "$devkitproWin\tools\bin\ndstool.exe" -f $romPath
    
    Write-Host "Compilation and patching complete!"
} else {
    Write-Error "Compilation failed! OveNotesDs.nds was not created."
}
