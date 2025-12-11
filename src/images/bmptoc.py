from PIL import Image
import sys
import os

inputfile = sys.argv[1]
imgname = sys.argv[2] if len(sys.argv) > 2 else os.path.basename(inputfile).rsplit('.', 1)[0]

img = Image.open(inputfile)

for i in range(1):
    pixels = img.load()  # load the pixel value

    output = []

    w, h = img.size
    for y in range(h):
        for x in range(w):
            if (y*w+x) % 8 == 0:
                output.append(0)
            pixel = pixels[x, y]

            bit = 1 if pixel == (255, 255, 255) else 0
            output[-1] = (bit << 7) | (output[-1] >> 1)
    output[-1] >>= 8 - (h*w % 8)
    print(f"""

// === {imgname.upper()} ===

static const uint8_t img_data_{imgname}[] = {{
    {" ".join(map(lambda x: f'0x{x:02X},', output))}
}};

const struct draw_img img_{imgname} = {{
    .width = {img.size[0]},
    .height = {img.size[1]},
    .data = img_data_{imgname},
}};""")

    print("\n// In header file: \n", file=sys.stderr)

    print(f"""extern const struct draw_img img_{imgname};""")

