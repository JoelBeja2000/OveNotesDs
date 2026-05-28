const fs = require('fs');
const jpeg = require('jpeg-js');

const filename = process.argv[2] || './pointer_sheep.png';
if (!fs.existsSync(filename)) {
    console.error(`Error: File not found: ${filename}\nUsage: node test_threshold.js <path_to_image_file>`);
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

function testThreshold(threshold) {
    const grid = [];
    for (let ty = 0; ty < targetHeight; ty++) {
        let row = '';
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
                        if (r > 90 || g > 90 || b > 80) {
                            val = 255;
                        }
                        sum += val;
                        count++;
                    }
                }
            }
            const avg = count > 0 ? sum / count : 0;
            if (avg >= threshold) {
                row += '█';
            } else {
                row += ' ';
            }
        }
        grid.push(row);
    }
    console.log(`=== THRESHOLD: ${threshold} ===`);
    grid.forEach(r => console.log(r));
    console.log('\n');
}

// Try a wide range of thresholds
for (let t = 20; t <= 180; t += 20) {
    testThreshold(t);
}
