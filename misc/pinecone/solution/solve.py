#!/usr/bin/env python3
import sys
from collections import deque
from PIL import Image, ImageFilter

PAIR = {
    "BR": "AB", "BLR": "CD", "BL": "EF",
    "TBR": "GH", "TBLR": "IJ", "TBL": "KL",
    "TR": "MN", "TLR": "OP", "TL": "RS",
}

def box(pts):
    xs, ys = zip(*pts)
    return min(xs), min(ys), max(xs) + 1, max(ys) + 1


def mask_components(mask, min_area):
    pix = mask.load()
    w, h = mask.size
    seen = set()
    for y in range(h):
        for x in range(w):
            if not pix[x, y] or (x, y) in seen:
                continue
            q = deque([(x, y)])
            seen.add((x, y))
            pts = []
            while q:
                px, py = q.popleft()
                pts.append((px, py))
                for nx, ny in ((px + 1, py), (px - 1, py), (px, py + 1), (px, py - 1)):
                    if 0 <= nx < w and 0 <= ny < h and pix[nx, ny] and (nx, ny) not in seen:
                        seen.add((nx, ny))
                        q.append((nx, ny))
            if len(pts) >= min_area:
                yield box(pts)


def point_components(points):
    todo = set(points)
    while todo:
        q = deque([todo.pop()])
        comp = []
        while q:
            x, y = q.popleft()
            comp.append((x, y))
            for nx in (x - 1, x, x + 1):
                for ny in (y - 1, y, y + 1):
                    if (nx, ny) in todo:
                        todo.remove((nx, ny))
                        q.append((nx, ny))
        yield comp


def classify(white, b):
    x0, y0, x1, y1 = b
    pts = [(x, y) for y in range(y0, y1) for x in range(x0, x1) if (x, y) in white]
    min_x, min_y, max_x, max_y = box(pts)

    dot = None
    for comp in point_components(pts):
        cx0, cy0, cx1, cy1 = box(comp)
        if 40 <= len(comp) <= 140 and cx1 - cx0 <= 20 and cy1 - cy0 <= 20:
            dot = (sum(x for x, _ in comp) / len(comp), sum(y for _, y in comp) / len(comp))
            break

    if dot is None:
        w, h = max_x - min_x + 1, max_y - min_y + 1
        if w > 65 and h > 65 and abs(w - h) < 10:
            return "Z"
        mid_y = (min_y + max_y) / 2
        xs = [x for x, y in pts if abs(y - mid_y) < 4]
        return "U" if sum(xs) / len(xs) > (min_x + max_x) / 2 else "Y"

    counts = (
        sum(y <= min_y + 5 for x, y in pts),
        sum(y >= max_y - 5 for x, y in pts),
        sum(x <= min_x + 5 for x, y in pts),
        sum(x >= max_x - 5 for x, y in pts),
    )
    frame = "".join(edge for edge, n in zip("TBLR", counts) if n > 35)
    return PAIR[frame][dot[0] > (min_x + max_x) / 2]


img = Image.open(sys.argv[1]).convert("RGBA")
white = {(x, y) for y in range(img.height) for x in range(img.width) if img.getpixel((x, y)) == (255, 255, 255, 255)}

mask = Image.new("L", img.size, 0)
for p in white:
    mask.putpixel(p, 255)

rows = []
for b in sorted(mask_components(mask.filter(ImageFilter.MaxFilter(41)), 2000), key=lambda b: (b[1], b[0])):
    cy = (b[1] + b[3]) / 2
    for row in rows:
        if abs(row[0] - cy) < 80:
            row[1].append(b)
            row[0] = (row[0] * (len(row[1]) - 1) + cy) / len(row[1])
            break
    else:
        rows.append([cy, [b]])

decoded = ["".join(classify(white, b) for b in sorted(row)) for _, row in rows]
flag = "".join(decoded).lower()

print("\n".join(decoded))
print(flag)
print(f"BtSCTF{{{flag}}}")
