# Yannsze created the script for transforming raw to C-array header file. 
# convert_raw_to_header_2.py
import numpy as np

# ======== SETTINGS =========
filename = "cat.raw"      # <-- your raw file
width = 256
height = 256
varname = "cat_img"       # C array variable name

data = np.fromfile(filename, dtype=np.uint8)

if data.size != width * height:
    raise ValueError(f"ERROR: File size {data.size} bytes doesn't match {width}x{height} = {width*height} bytes")

data = data.reshape((height, width))

with open("cat_image.h", "w") as f:
    f.write("#ifndef CAT_IMAGE_H\n#define CAT_IMAGE_H\n\n")
    f.write(f"static const unsigned char {varname}[{height}][{width}] = {{\n")
    for row in data:
        f.write("    { " + ", ".join(map(str, row)) + " },\n")
    f.write("};\n\n#endif // CAT_IMAGE_H\n")

print("Done! Generated cat_image.h")
