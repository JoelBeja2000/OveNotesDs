const fs = require('fs');
const jpeg = require('jpeg-js');
const PNG = require('pngjs').PNG;

const filename = 'C:\\Users\\JOel\\.gemini\\antigravity\\brain\\7b50d8f8-2d81-4874-a1d8-4c0c8f501420\\ovenotes_logo_sheep_1779740660384.png';
const data = fs.readFileSync(filename);

let width, height, pixels;

if (data[0] === 0xFF && data[1] === 0xD8) {
    console.log("Detected JPEG format");
    const decoded = jpeg.decode(data, { useTArray: true });
    width = decoded.width;
    height = decoded.height;
    pixels = decoded.data;
} else {
    console.log("Detected PNG or other format");
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

const targetWidth = 256;
const targetHeight = 192;
const grayscale = new Uint8Array(targetWidth * targetHeight);

for (let y = 0; y < targetHeight; y++) {
    for (let x = 0; x < targetWidth; x++) {
        const srcX = Math.floor((x / targetWidth) * width);
        const srcY = Math.floor((y / targetHeight) * height);
        const idx = (srcY * width + srcX) * 4;
        
        const r = pixels[idx];
        const g = pixels[idx + 1];
        const b = pixels[idx + 2];
        
        const gray = Math.round(0.299 * r + 0.587 * g + 0.114 * b);
        grayscale[y * targetWidth + x] = gray;
    }
}

let out = `/* Auto-generated logo data from ovenotes_logo_banner */\n`;
out += `#ifndef LOGO_DATA_H\n`;
out += `#define LOGO_DATA_H\n\n`;
out += `const unsigned char logo_data[256 * 192] = {\n`;

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
out += `#endif /* LOGO_DATA_H */\n`;

fs.writeFileSync('source\\logo_data.h', out);
console.log('Successfully generated logo_data.h!');
