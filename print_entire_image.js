const fs = require('fs');
const jpeg = require('jpeg-js');

const filename = 'C:\\Users\\JOel\\.gemini\\antigravity\\brain\\3c35ccee-0312-4f9d-970f-b0f54b1ff54d\\pointer_sheep_sharp_1779837050861.png';
const data = fs.readFileSync(filename);
const decoded = jpeg.decode(data, { useTArray: true });
const width = decoded.width;
const height = decoded.height;
const pixels = decoded.data;

const targetWidth = 64;
const targetHeight = 64;

console.log("Entire image grid:");
for (let ty = 0; ty < targetHeight; ty++) {
    let row = '';
    for (let tx = 0; tx < targetWidth; tx++) {
        const srcX = Math.floor((tx / targetWidth) * width);
        const srcY = Math.floor((ty / targetHeight) * height);
        const idx = (srcY * width + srcX) * 4;
        const r = pixels[idx];
        const g = pixels[idx+1];
        const b = pixels[idx+2];
        row += (r > 30 || g > 30 || b > 30) ? '#' : ' ';
    }
    console.log(row);
}
