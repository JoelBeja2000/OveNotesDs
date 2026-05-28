$romPath = Join-Path $PSScriptRoot "OveNotesDs.nds"
if (-not (Test-Path $romPath)) {
    Write-Error "OveNotesDs.nds not found in script directory."
    exit 1
}
$b = [System.IO.File]::ReadAllBytes($romPath)
Write-Output ("Unit Code (0x12): 0x{0:X2}" -f $b[0x12])
Write-Output ("DSi Region (0x1B0): 0x{0:X8}" -f [System.BitConverter]::ToUInt32($b, 0x1B0))
Write-Output ("DSi TitleID lo (0x230): 0x{0:X8}" -f [System.BitConverter]::ToUInt32($b, 0x230))
Write-Output ("DSi TitleID hi (0x234): 0x{0:X8}" -f [System.BitConverter]::ToUInt32($b, 0x234))
Write-Output ("ARM9i offset (0x1C0): 0x{0:X8}" -f [System.BitConverter]::ToUInt32($b, 0x1C0))
Write-Output ("ARM7i offset (0x1D0): 0x{0:X8}" -f [System.BitConverter]::ToUInt32($b, 0x1D0))
Write-Output ("Total size: {0}" -f $b.Length)
Write-Output ("Header size bytes 0x80-0x83: 0x{0:X8}" -f [System.BitConverter]::ToUInt32($b, 0x80))
