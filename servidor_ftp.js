const FtpSrv = require('ftp-srv');
const path = require('path');
const fs = require('fs');

// Create the directory where notes will be saved if it doesn't exist
const notesDir = path.join(__dirname, 'Notas_Publicadas');
if (!fs.existsSync(notesDir)) {
    fs.mkdirSync(notesDir);
}

// Ensure the local IP matches what we hardcoded in OveNotesDs
const ftpServer = new FtpSrv({
    url: 'ftp://0.0.0.0:21',
    pasv_url: '192.168.1.132',
    anonymous: true,
    greeting: ['Bienvenido al servidor FTP de OveNotes DS!']
});

ftpServer.on('login', ({ connection, username, password }, resolve, reject) => {
    // Accept the default credentials from the app
    if (username === 'anonymous' && password === 'ds@ds.com') {
        return resolve({ root: notesDir });
    }
    return reject(new Error('Credenciales incorrectas'));
});

ftpServer.listen().then(() => {
    console.log('===================================================');
    console.log('🚀 Servidor FTP iniciado correctamente!');
    console.log('📡 Escuchando en la IP: 192.168.1.132, Puerto: 21');
    console.log(`📁 Las notas se guardarán en: ${notesDir}`);
    console.log('===================================================');
    console.log('Ya puedes darle al botón "PUBLICAR" en tu Nintendo DSi.');
});
