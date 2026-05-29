const http = require('http');
const fs = require('fs');
const path = require('path');
const os = require('os');

const PORT = 3000;
const NOTES_DIR = path.join(__dirname, 'Notas_Publicadas');

if (!fs.existsSync(NOTES_DIR)) {
    fs.mkdirSync(NOTES_DIR);
}

// ─── Pairing Code System ───────────────────────────────────────────
// Encodes the server's local IP + port into a short Base36 code
// so the DS user doesn't have to type the IP and port manually.

const BASE36_CHARS = '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ';

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

function encodePairingCode(ip, port) {
    const parts = ip.split('.').map(Number);
    // Pack IP (4 bytes) + port (2 bytes) into a single BigInt (48 bits)
    let value = BigInt(0);
    for (let i = 0; i < 4; i++) {
        value = (value << BigInt(8)) | BigInt(parts[i]);
    }
    value = (value << BigInt(16)) | BigInt(port);

    // Convert to Base36
    let code = '';
    while (value > BigInt(0)) {
        code = BASE36_CHARS[Number(value % BigInt(36))] + code;
        value = value / BigInt(36);
    }

    // Pad to 10 characters
    while (code.length < 10) {
        code = '0' + code;
    }
    return code;
}

const LOCAL_IP = getLocalIP();
const PAIRING_CODE = encodePairingCode(LOCAL_IP, PORT);

function restartServer() {
    const spawn = require('child_process').spawn;
    const process = require('process');
    
    console.log('🔄 Reiniciando el servidor...');
    
    const child = spawn(process.argv[0], process.argv.slice(1), {
        cwd: process.cwd(),
        detached: true,
        stdio: 'ignore'
    });
    
    child.unref();
    process.exit(0);
}

// ────────────────────────────────────────────────────────────────────

const server = http.createServer((req, res) => {
    // Enable CORS
    res.setHeader('Access-Control-Allow-Origin', '*');
    res.setHeader('Access-Control-Allow-Methods', 'GET, POST, OPTIONS, DELETE');
    res.setHeader('Access-Control-Allow-Headers', 'Content-Type');

    if (req.method === 'OPTIONS') {
        res.writeHead(204);
        res.end();
        return;
    }

    // Serve manifest.json
    if (req.url === '/manifest.json' && req.method === 'GET') {
        fs.readFile(path.join(__dirname, 'dist', 'manifest.json'), (err, data) => {
            if (err) {
                res.writeHead(404);
                res.end();
                return;
            }
            res.writeHead(200, { 'Content-Type': 'application/json' });
            res.end(data);
        });
        return;
    }

    // Serve logo_pwa.png
    if (req.url === '/logo_pwa.png' && req.method === 'GET') {
        fs.readFile(path.join(__dirname, 'dist', 'logo_pwa.png'), (err, data) => {
            if (err) {
                res.writeHead(404);
                res.end();
                return;
            }
            res.writeHead(200, { 'Content-Type': 'image/png' });
            res.end(data);
        });
        return;
    }

    // Serve sw.js
    if (req.url === '/sw.js' && req.method === 'GET') {
        fs.readFile(path.join(__dirname, 'dist', 'sw.js'), (err, data) => {
            if (err) {
                res.writeHead(404);
                res.end();
                return;
            }
            res.writeHead(200, { 'Content-Type': 'application/javascript' });
            res.end(data);
        });
        return;
    }

    // Serve HTML Client
    if (req.url === '/' && req.method === 'GET') {
        fs.readFile(path.join(__dirname, 'dist', 'index.html'), 'utf8', (err, data) => {
            if (err) {
                res.writeHead(500, { 'Content-Type': 'text/plain' });
                res.end('Internal Server Error');
                return;
            }
            // Inject initial pairing code into the served HTML so the UI shows it immediately
            try {
                const injectionToken = "let pairingCodeData = { code: '------', ip: '...', port: '...' };";
                const injected = `let pairingCodeData = { code: '${PAIRING_CODE}', ip: '${LOCAL_IP}', port: ${PORT} };`;
                if (data.includes(injectionToken)) {
                    data = data.replace(injectionToken, injected);
                } else {
                    // Fallback: append a small script with initial pairing data before </head>
                    const scriptTag = `<script>window.initialPairing = { code: '${PAIRING_CODE}', ip: '${LOCAL_IP}', port: ${PORT} };</script>`;
                    data = data.replace('</head>', scriptTag + '\n</head>');
                }
                // Inject cached notes list so thumbnails appear immediately
                try {
                    const notesFiles = fs.readdirSync(NOTES_DIR).filter(f => f.endsWith('.png'))
                        .map(f => {
                            const stats = fs.statSync(path.join(NOTES_DIR, f));
                            return { name: f, time: stats.mtimeMs };
                        })
                        .sort((a,b) => b.time - a.time);

                    const cachedToken = "let cachedNotes = [];";
                    const cachedInjected = `let cachedNotes = ${JSON.stringify(notesFiles)};`;
                    if (data.includes(cachedToken)) {
                        data = data.replace(cachedToken, cachedInjected);
                    } else {
                        const scriptTag2 = `<script>window.initialNotes = ${JSON.stringify(notesFiles)};</script>`;
                        data = data.replace('</head>', scriptTag2 + '\n</head>');
                    }
                } catch (e) {
                    console.error('Notes injection failed', e);
                }
            } catch (e) {
                console.error('Pairing injection failed', e);
            }

            res.writeHead(200, { 'Content-Type': 'text/html' });
            res.end(data);
        });
        return;
    }

    // Pairing code endpoint
    if (req.url === '/api/pairing-code' && req.method === 'GET') {
        res.writeHead(200, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify({ code: PAIRING_CODE, ip: LOCAL_IP, port: PORT }));
        return;
    }

    // Handle incoming note uploads from Nintendo DS (raw binary PNG)
    if (req.url === '/api/nueva-nota' && req.method === 'POST') {
        let body = [];
        req.on('data', chunk => {
            body.push(chunk);
        });
        req.on('end', () => {
            const buffer = Buffer.concat(body);
            const filename = `nota_${Date.now()}.png`;
            const filepath = path.join(NOTES_DIR, filename);

            fs.writeFile(filepath, buffer, err => {
                if (err) {
                    console.error('Error saving note:', err);
                    res.writeHead(500, { 'Content-Type': 'application/json' });
                    res.end(JSON.stringify({ success: false, error: 'Could not save note' }));
                    return;
                }
                console.log(`📝 Nueva nota recibida y guardada: ${filename}`);
                res.writeHead(201, { 'Content-Type': 'application/json' });
                res.end(JSON.stringify({ success: true, filename }));
            });
        });
        return;
    }

    // List all uploaded notes (sorted newest first)
    if (req.url === '/api/notas' && req.method === 'GET') {
        fs.readdir(NOTES_DIR, (err, files) => {
            if (err) {
                res.writeHead(500, { 'Content-Type': 'application/json' });
                res.end(JSON.stringify({ success: false, error: 'Could not read directory' }));
                return;
            }
            const notes = files
                .filter(file => file.endsWith('.png'))
                .map(file => {
                    const filepath = path.join(NOTES_DIR, file);
                    const stats = fs.statSync(filepath);
                    return {
                        name: file,
                        time: stats.mtimeMs
                    };
                })
                .sort((a, b) => b.time - a.time);

            res.writeHead(200, { 'Content-Type': 'application/json' });
            res.end(JSON.stringify(notes));
        });
        return;
    }

    // Serve static image files
    if (req.url.startsWith('/notas/') && req.method === 'GET') {
        const cleanUrl = req.url.split('?')[0];
        const filename = path.basename(cleanUrl);
        const filepath = path.join(NOTES_DIR, filename);
        const isDownload = req.url.includes('download=1');

        if (fs.existsSync(filepath)) {
            fs.readFile(filepath, (err, data) => {
                if (err) {
                    res.writeHead(500, { 'Content-Type': 'text/plain' });
                    res.end('Error reading image');
                    return;
                }
                const headers = { 'Content-Type': 'image/png' };
                if (isDownload) {
                    headers['Content-Disposition'] = `attachment; filename="${filename}"`;
                }
                res.writeHead(200, headers);
                res.end(data);
            });
        } else {
            res.writeHead(404, { 'Content-Type': 'text/plain' });
            res.end('Not Found');
        }
        return;
    }

    // Delete note
    if (req.url.startsWith('/api/notas/') && req.method === 'DELETE') {
        const filename = path.basename(req.url);
        const filepath = path.join(NOTES_DIR, filename);
        if (fs.existsSync(filepath)) {
            fs.unlink(filepath, err => {
                if (err) {
                    res.writeHead(500, { 'Content-Type': 'application/json' });
                    res.end(JSON.stringify({ success: false, error: 'Could not delete file' }));
                    return;
                }
                console.log(`🗑️ Nota eliminada: ${filename}`);
                res.writeHead(200, { 'Content-Type': 'application/json' });
                res.end(JSON.stringify({ success: true }));
            });
        } else {
            res.writeHead(404, { 'Content-Type': 'application/json' });
            res.end(JSON.stringify({ success: false, error: 'File not found' }));
        }
        return;
    }

    // Restart server endpoint
    if (req.url === '/api/restart' && req.method === 'POST') {
        res.writeHead(200, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify({ success: true, message: 'Server is restarting...' }));
        setTimeout(() => {
            restartServer();
        }, 1000);
        return;
    }

    res.writeHead(404, { 'Content-Type': 'text/plain' });
    res.end('Not Found');
});

server.listen(PORT, '0.0.0.0', () => {
    console.log('===================================================');
    console.log('🚀 Servidor HTTP iniciado correctamente!');
    console.log(`📡 Escuchando en el Puerto: ${PORT}`);
    console.log(`🌐 IP Local: ${LOCAL_IP}`);
    console.log(`🔗 Código de emparejamiento: ${PAIRING_CODE}`);
    console.log(`📁 Las notas se guardarán en: ${NOTES_DIR}`);
    console.log(`🌐 Abre http://localhost:${PORT}/ en tu navegador`);
    console.log('===================================================');
});
