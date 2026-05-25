const http = require('http');
const fs = require('fs');
const path = require('path');

const PORT = 3000;
const NOTES_DIR = path.join(__dirname, 'Notas_Publicadas');

if (!fs.existsSync(NOTES_DIR)) {
    fs.mkdirSync(NOTES_DIR);
}

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

    // Serve HTML Client
    if (req.url === '/' && req.method === 'GET') {
        fs.readFile(path.join(__dirname, 'index.html'), 'utf8', (err, data) => {
            if (err) {
                res.writeHead(500, { 'Content-Type': 'text/plain' });
                res.end('Internal Server Error');
                return;
            }
            res.writeHead(200, { 'Content-Type': 'text/html' });
            res.end(data);
        });
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
        const filename = path.basename(req.url);
        const filepath = path.join(NOTES_DIR, filename);
        if (fs.existsSync(filepath)) {
            fs.readFile(filepath, (err, data) => {
                if (err) {
                    res.writeHead(500, { 'Content-Type': 'text/plain' });
                    res.end('Error reading image');
                    return;
                }
                res.writeHead(200, { 'Content-Type': 'image/png' });
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

    res.writeHead(404, { 'Content-Type': 'text/plain' });
    res.end('Not Found');
});

server.listen(PORT, '0.0.0.0', () => {
    console.log('===================================================');
    console.log('🚀 Servidor HTTP iniciado correctamente!');
    console.log(`📡 Escuchando en el Puerto: ${PORT}`);
    console.log(`📁 Las notas se guardarán en: ${NOTES_DIR}`);
    console.log(`🌐 Abre http://localhost:${PORT}/ en tu navegador`);
    console.log('===================================================');
});
