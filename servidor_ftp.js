const FtpSrv = require('ftp-srv');
const path = require('path');
const fs = require('fs');
const os = require('os');

// Get the local IP address dynamically
function getLocalIP() {
    const interfaces = os.networkInterfaces();
    for (const name of Object.keys(interfaces)) {
        for (const iface of interfaces[name]) {
            if (iface.family === 'IPv4' && !iface.internal) {
                return iface.address;
            }
        }
    }
    return '127.0.0.1';
}

const LOCAL_IP = getLocalIP();

// Create the directory where notes will be saved if it doesn't exist
const notesDir = path.join(__dirname, 'Notas_Publicadas');
if (!fs.existsSync(notesDir)) {
    fs.mkdirSync(notesDir);
}

// Ensure the local IP matches what we hardcoded in OveNotesDs
const ftpServer = new FtpSrv({
    url: 'ftp://0.0.0.0:21',
    pasv_url: LOCAL_IP,
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
    console.log(`📡 Escuchando en la IP: ${LOCAL_IP}, Puerto: 21`);
    console.log(`📁 Las notas se guardarán en: ${notesDir}`);
    console.log('===================================================');
    console.log('Ya puedes darle al botón "PUBLICAR" en tu Nintendo DSi.');
});
