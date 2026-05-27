const fs = require('fs');
const jpeg = require('jpeg-js');
const PNG = require('pngjs').PNG;

const filename = 'C:\\Users\\JOel\\.gemini\\antigravity\\brain\\3c35ccee-0312-4f9d-970f-b0f54b1ff54d\\pointer_sheep_sharp_1779837050861.png';
const data = fs.readFileSync(filename);

let width, height, pixels;

if (data[0] === 0xFF && data[1] === 0xD8) {
    console.log("Detected JPEG format");
    const decoded = jpeg.decode(data, { useTArray: true });
    width = decoded.width;
    height = decoded.height;
    pixels = decoded.data;
} else {
    console.log("Detected PNG format");
    let pngBuffer = data;
    try {
        const png = PNG.sync.read(pngBuffer);
        width = png.width;
        height = png.height;
        pixels = png.data;
    } catch (e) {
        console.warn("PNG parsing failed. Attempting to slice PNG buffer to IEND chunk...", e.message);
        const iendIndex = data.indexOf(Buffer.from('IEND'));
        if (iendIndex !== -1) {
            pngBuffer = data.slice(0, iendIndex + 8);
            const png = PNG.sync.read(pngBuffer);
            width = png.width;
            height = png.height;
            pixels = png.data;
        } else {
            throw e;
        }
    }
}

// Bounding box of the sheep only:
// minX: 163, maxX: 911 (width = 748)
// minY: 286, maxY: 952 (height = 666)
const minX = 163;
const maxX = 911;
const minY = 286;
const maxY = 952;

const sheepWidth = maxX - minX;   // 748
const sheepHeight = maxY - minY;  // 666
const cropSize = sheepWidth;      // 748 (use the larger dimension for square crop)
const padY = Math.floor((cropSize - sheepHeight) / 2); // 41 pixels padding at top/bottom

const targetWidth = 48;
const targetHeight = 48;
const rawGrid = new Uint8Array(targetWidth * targetHeight);

const colorThreshold = 50;
const boxThreshold = 120;

// Step 1: Average Box Filtering
for (let ty = 0; ty < targetHeight; ty++) {
    for (let tx = 0; tx < targetWidth; tx++) {
        const startX = Math.floor(minX + (tx / targetWidth) * cropSize);
        const endX = Math.floor(minX + ((tx + 1) / targetWidth) * cropSize);
        const startY = Math.floor(minY + (ty / targetHeight) * cropSize - padY);
        const endY = Math.floor(minY + ((ty + 1) / targetHeight) * cropSize - padY);
        
        let sum = 0;
        let count = 0;
        for (let sy = startY; sy < endY; sy++) {
            for (let sx = startX; sx < endX; sx++) {
                if (sx >= minX && sx <= maxX && sy >= minY && sy <= maxY) {
                    const idx = (sy * width + sx) * 4;
                    const r = pixels[idx];
                    const g = pixels[idx+1];
                    const b = pixels[idx+2];
                    let val = 0;
                    if (r > colorThreshold || g > colorThreshold || b > colorThreshold) {
                        val = 255;
                    }
                    sum += val;
                    count++;
                }
            }
        }
        const avg = count > 0 ? sum / count : 0;
        rawGrid[ty * targetWidth + tx] = (avg >= boxThreshold) ? 255 : 0;
    }
}

// Step 2: Connected Component Analysis to filter out stray noise
const visited = new Uint8Array(targetWidth * targetHeight);
const grayscale = new Uint8Array(targetWidth * targetHeight);
const components = [];

for (let y = 0; y < targetHeight; y++) {
    for (let x = 0; x < targetWidth; x++) {
        const idx = y * targetWidth + x;
        if (visited[idx] || rawGrid[idx] === 0) continue;

        const comp = [];
        const queue = [[x, y]];
        visited[idx] = 1;

        while (queue.length > 0) {
            const [cx, cy] = queue.shift();
            comp.push([cx, cy]);

            const neighbors = [
                [cx-1, cy], [cx+1, cy],
                [cx, cy-1], [cx, cy+1],
                [cx-1, cy-1], [cx+1, cy-1],
                [cx-1, cy+1], [cx+1, cy+1]
            ];
            for (const [nx, ny] of neighbors) {
                if (nx >= 0 && nx < targetWidth && ny >= 0 && ny < targetHeight) {
                    const nidx = ny * targetWidth + nx;
                    if (!visited[nidx] && rawGrid[nidx] > 0) {
                        visited[nidx] = 1;
                        queue.push([nx, ny]);
                    }
                }
            }
        }
        components.push(comp);
    }
}

// Keep components of size >= 4
components.forEach(c => {
    if (c.length >= 4) {
        c.forEach(([x, y]) => {
            grayscale[y * targetWidth + x] = 255;
        });
    }
});

let out = `/* Auto-generated pointer sheep data from pointer_sheep PNG */\n`;
out += `#ifndef POINTER_SHEEP_DATA_H\n`;
out += `#define POINTER_SHEEP_DATA_H\n\n`;
out += `const unsigned char pointer_sheep_data[48 * 48] = {\n`;

for (let i = 0; i < grayscale.length; i += 16) {
    const chunk = Array.from(grayscale.slice(i, i + 16));
    const hex = chunk.map(v => `0x${v.toString(16).padStart(2, '0').toUpperCase()}`).join(', ');
    if (i + 16 < grayscale.length) {
        out += `    ${hex},\n`;
    } else {
        out += `    ${hex}\n`;
    }
}
out += `};\n\n`;
out += `#endif /* POINTER_SHEEP_DATA_H */\n`;

fs.writeFileSync('source\\pointer_sheep_data.h', out);
console.log('Successfully generated pointer_sheep_data.h!');
