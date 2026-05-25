function Dump-Header($path) {
    if (Test-Path $path) {
        Write-Output "=== Header for $path ==="
        $bytes = [System.IO.File]::ReadAllBytes($path)
        Write-Output ("Title:       " + [System.Text.Encoding]::ASCII.GetString($bytes[0..11]))
        Write-Output ("Game Code:   " + [System.Text.Encoding]::ASCII.GetString($bytes[12..15]))
        Write-Output ("Maker Code:  " + [System.Text.Encoding]::ASCII.GetString($bytes[0x10..0x11]))
        Write-Output ("Unit Code:   0x{0:X2}" -f $bytes[0x12])
        Write-Output ("Header CRC:  0x{0:X4}" -f [System.BitConverter]::ToUInt16($bytes, 0x15c))
        $size = (Get-Item $path).Length
        Write-Output ("File Size:   $size bytes")
    } else {
        Write-Output "File not found: $path"
    }
    Write-Output ""
}

Dump-Header "D:\Proyectos\OveNotesDs\OveNotesDs.nds"
Dump-Header "F:\roms\nds\OveNotesDs.nds"
Dump-Header "F:\OveNotesDs.nds"
