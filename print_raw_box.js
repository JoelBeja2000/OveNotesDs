const fs = require('fs');
const jpeg = require('jpeg-js');

const filename = 'C:\\Users\\JOel\\.gemini\\antigravity\\brain\\3c35ccee-0312-4f9d-970f-b0f54b1ff54d\\pointer_sheep_sharp_1779837050861.png';
const data = fs.readFileSync(filename);
const decoded = jpeg.decode(data, { useTArray: true });
const width = decoded.width;
const height = decoded.height;
const pixels = decoded.data;

const minX = 163;
const maxX = 911;
const minY = 286;
const maxY = 952;

const sheepWidth = maxX - minX;
const sheepHeight = maxY - minY;
const cropSize = sheepWidth;
const padY = Math.floor((cropSize - sheepHeight) / 2);

const targetWidth = 48;
const targetHeight = 48;

// We will print the thresholded values in a 48x48 grid to console
// We will print:
// '.' for background (color <= threshold)
// '#' for sheep (color > threshold)
// Let's print using the exact logic from process_pointer_sheep.js
for (let ty = 0; ty < targetHeight; ty++) {
    let row = '';
    for (let tx = 0; tx < targetWidth; tx++) {
        const srcX = Math.floor(minX + (tx / targetWidth) * cropSize);
        const srcY = Math.floor(minY + (ty / targetHeight) * cropSize - padY);
        
        let isYellow = false;
        if (srcX >= minX && srcX <= maxX && srcY >= minY && srcY <= maxY) {
            const idx = (srcY * width + srcX) * 4;
            const r = pixels[idx];
            const g = pixels[idx + 1];
            const b = pixels[idx + 2];
            if (r > 90 || g > 90 || b > 80) {
                isYellow = true;
            }
        }
        row += isYellow ? '#' : ' ';
    }
    console.log(row);
}
