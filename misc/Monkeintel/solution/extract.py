#!/usr/bin/env python3
import base64
import sys
from PIL import Image

img = Image.open(sys.argv[1]).convert("RGB")
pixels = img.get_flattened_data()
values = [channel for pixel in pixels for channel in pixel]
bits = [value & 1 for value in values[::2]]
data = bytes(
    int("".join(map(str, bits[i : i + 8])), 2)
    for i in range(0, len(bits) - 7, 8)
)
print(base64.b64decode(data.split(b"\0", 1)[0]).decode())
