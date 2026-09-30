"""Compile the real C renderer on the host and render 90s-theme previews.

Usage: python tools/preview_f1_90s.py OUTPUT_DIR
Produces race_90s_plate.png, race_90s_difference.png, qualy_90s.png and race_modern.png.
The palette and background are synthetic, NOT a GP2 capture: in the game the
colours are mapped to the nearest entries of GP2's own 256-colour palette.
"""
import ctypes as C, json, subprocess, sys, tempfile
from pathlib import Path
from PIL import Image, ImageDraw

root = Path(__file__).resolve().parents[1]
out_dir = Path(sys.argv[1] if len(sys.argv) > 1 else ".")
out_dir.mkdir(parents=True, exist_ok=True)
drivers = json.loads((root / "f1-input/drivers.json").read_text())["drivers"]
ids = [d["car_id"] for d in drivers]

class Row(C.Structure):
    _fields_ = [(k, C.c_int) for k in ["id", "pos", "lap", "out", "pit", "focused"]] + [
        ("gap", C.c_long), ("gain", C.c_int), ("stops", C.c_int), ("leaderGap", C.c_long),
        ("lapsBehind", C.c_int), ("fastest", C.c_int), ("positionChange", C.c_int)]

class QRow(C.Structure):
    _fields_ = [(k, C.c_int) for k in ["id", "pos", "pit", "outlap"]] + [
        ("best", C.c_long), ("pb", C.c_long * 3), ("live", C.c_long * 2), ("color", C.c_int * 2)]

class QCard(C.Structure):
    _fields_ = [(k, C.c_int) for k in ["id", "pos", "pit", "outlap", "post", "pole", "showGap", "hasGap", "leaderId"]] + [
        ("running", C.c_long), ("last", C.c_long), ("gap", C.c_long), ("leaderBest", C.c_long), ("color", C.c_int * 3)]

def synthetic_palette():
    pal = []
    for r in range(6):
        for g in range(7):
            for b in range(6):
                pal += [round(r * 63 / 5), round(g * 63 / 6), round(b * 63 / 5)]
    for v in (10, 21, 42, 53):
        pal += [v, v, v]
    return pal

def background(pal):
    im = Image.new("RGB", (640, 480))
    d = ImageDraw.Draw(im)
    for y in range(480):
        if y < 190:
            t = y / 190; d.line([(0, y), (639, y)], fill=(int(110 + 60 * t), int(150 + 50 * t), int(200 + 30 * t)))
        else:
            d.line([(0, y), (639, y)], fill=(58, 112, 52))
    d.polygon([(250, 190), (390, 190), (640, 480), (0, 480)], fill=(78, 80, 84))
    d.polygon([(0, 150), (640, 120), (640, 190), (0, 190)], fill=(120, 124, 132))
    pimg = Image.new("P", (1, 1)); pimg.putpalette([v * 4 for v in pal])
    q = im.quantize(palette=pimg, dither=Image.Dither.NONE)
    return bytearray(q.tobytes())

def save(buf, pal, name):
    # TV view: GP2 only displays lines 0..387, the rest is shown here as black.
    for i in range(388 * 640, 480 * 640):
        buf[i] = 0
    img = Image.frombytes("P", (640, 480), bytes(buf)); img.putpalette([v * 4 for v in pal])
    img.convert("RGB").resize((1280, 960), Image.Resampling.NEAREST).save(out_dir / name)
    print("wrote", out_dir / name)

with tempfile.TemporaryDirectory() as temp:
    lib = Path(temp) / "render.so"
    subprocess.run(["gcc", "-std=c89", "-Wall", "-Wextra", "-Werror", "-Wno-missing-braces", "-shared", "-fPIC",
                    str(root / "src/f1render.c"), str(root / "src/f1timing.c"), str(root / "src/f1config.c"),
                    "-o", str(lib)], check=True)
    L = C.CDLL(str(lib))
    L.F1ConfigEnsure()
    theme = C.c_int.in_dll(L, "f1_theme")
    pal = synthetic_palette(); cpal = (C.c_ubyte * 768)(*pal)
    U8 = C.c_ubyte * (640 * 480)

    # ── race fixture: 26 cars, focus P6, pit P20, retirement P26 ──
    gaps = [0, 1284, 917, 2406, 533, 717, 1052, 388, 3117, 846, 1573, 402, 2019, 694, 1238, 557,
            4301, 973, 1415, 2886, 612, 1947, 735, 3502, 1210, -1]
    rows = (Row * 26)()
    for i in range(26):
        r = rows[i]; r.id = ids[i]; r.pos = i + 1; r.lap = 12; r.gap = gaps[i]; r.leaderGap = sum(gaps[1:i + 1])
        r.focused = i == 5; r.pit = i == 19; r.out = i == 25; r.gain = 1000; r.stops = 1 if i in (3, 19) else 0
        r.positionChange = 1 if i == 8 else (-1 if i == 9 else 0)
    focus = ids[5]

    # TV view, 90s: TAB selects plate / DIFFERENCE / nothing in the bottom slot
    for name, t, slot in (("race_90s_plate.png", 1, "plate"), ("race_90s_difference.png", 1, "diff"),
                          ("race_modern.png", 0, "plate")):
        theme.value = t
        buf = U8(*background(pal))
        L.F1RenderRace(buf, cpal, rows, 26, 12, 43, 0)
        if slot == "plate":
            L.F1RenderDriverAtXY(buf, cpal, focus, 6, L.F1DriverLeft(0), L.F1DriverTop(0, 0, 26))
        else:
            L.F1Render90Difference(buf, cpal, rows, 26, focus)
        if t:
            L.F1Render90Fastest(buf, cpal, ids[0], C.c_long(82207), 8)
        save(buf, pal, name)

    # ── qualy fixture ──
    theme.value = 1
    qrows = (QRow * 26)()
    base = 82207
    for i in range(26):
        q = qrows[i]; q.id = ids[i]; q.pos = i + 1
        q.best = 0 if i >= 23 else base + i * 137 + (i * i * 11)
        q.pit = i in (4, 23)
    card = QCard(id=ids[5], pos=6, post=0, showGap=1, hasGap=1, leaderId=ids[0], running=61234,
                 last=83011, gap=-214, leaderBest=base)
    card.color[0] = 3; card.color[1] = 1; card.color[2] = 0
    buf = U8(*background(pal))
    L.F1RenderQualy(buf, cpal, qrows, 26, C.c_long(754000), 0, 1, 0, ids[5])
    L.F1RenderQualyCard(buf, cpal, C.byref(card), 0)
    L.F1RenderDriverAtXY(buf, cpal, ids[5], 6, L.F1DriverLeft(0), L.F1DriverTop(0, 1, 26))
    save(buf, pal, "qualy_90s.png")
