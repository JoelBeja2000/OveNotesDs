const fs = require('fs');
const jpeg = require('jpeg-js');

const filename = process.argv[2] || './pointer_sheep.png';
if (!fs.existsSync(filename)) {
    console.error(`Error: File not found: ${filename}\nUsage: node print_raw_box.js <path_to_image_file>`);
    process.exit(1);
}
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
