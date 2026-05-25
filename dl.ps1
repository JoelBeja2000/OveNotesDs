[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
$response = Invoke-RestMethod 'https://api.github.com/repos/devkitPro/nds-hb-menu/releases/latest'
$url = $response.assets[0].browser_download_url
Invoke-WebRequest -Uri $url -OutFile 'F:\hbmenu.tar.bz2'
Write-Host "Downloaded to F:\hbmenu.tar.bz2"
