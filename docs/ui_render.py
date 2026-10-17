#!/usr/bin/env python3
"""Render the Clock32 faces to PNG, for the UI documentation.

The firmware draws into a 128x64 mono u8g2 buffer with a handful of primitives
(box, frame, string) and three colours (0 = clear, 1 = set, 2 = XOR).  This
script re-implements just those primitives - including the u8g2 font format, the
RLE glyph decoder and rotated text - so the screens can be reproduced off-target
and documented without a display or a debugger.

Every coordinate below is copied from ``Core/Src/display_draw.c``; if a layout
changes there, update the matching function here and re-run:

    python docs/ui_render.py

Requires Python 3.8+ and nothing else (PNG is written with zlib).
"""

from __future__ import annotations

import os
import re
import struct
import zlib

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
FONT_SOURCE = os.path.join(ROOT, "Drivers", "u8g2", "csrc", "u8g2_fonts.c")
IMAGE_DIR = os.path.join(HERE, "images")

SCALE = 3            # 128x64 -> 384x192 px
W, H = 128, 64


# --------------------------------------------------------------------------- fonts
def _extract_font(name: str) -> bytes:
    """Pull one font array out of u8g2_fonts.c (octal-escaped C string literals)."""
    with open(FONT_SOURCE, "rb") as f:
        src = f.read().decode("utf-8", "replace")
    m = re.search(r"const uint8_t " + name + r"\[\d+\][^=]*=\s*", src)
    i, out = m.end(), bytearray()
    while src[i] != ";":
        if src[i] != '"':
            i += 1
            continue
        i += 1
        while src[i] != '"':
            if src[i] == "\\":
                i += 1
                n = src[i]
                if n in "01234567":
                    digits = n
                    i += 1
                    for _ in range(2):
                        if src[i] in "01234567":
                            digits += src[i]
                            i += 1
                        else:
                            break
                    out.append(int(digits, 8) & 0xFF)
                    continue
                if n == "x":
                    i += 1
                    hexa = ""
                    while src[i] in "0123456789abcdefABCDEF":
                        hexa += src[i]
                        i += 1
                    out.append(int(hexa, 16) & 0xFF)
                    continue
                out.append({"n": 10, "t": 9, "r": 13, "\\": 92, '"': 34, "'": 39,
                            "a": 7, "b": 8, "f": 12, "v": 11, "?": 63}[n])
                i += 1
                continue
            out.append(ord(src[i]) & 0xFF)
            i += 1
        i += 1
    return bytes(out)


class _Bits:
    """u8g2_font_decode_get_unsigned_bits / _signed_bits."""

    def __init__(self, data: bytes):
        self.d, self.pos, self.bit = data, 0, 0

    def u(self, cnt: int) -> int:
        if cnt == 0:
            return 0
        i, val, bp = self.pos, self.d[self.pos], self.bit
        val >>= bp
        total = bp + cnt
        if total >= 8:
            i += 1
            val |= self.d[i] << (8 - bp)
            total -= 8
        val &= (1 << cnt) - 1
        self.pos, self.bit = i, total
        return val

    def s(self, cnt: int) -> int:
        return self.u(cnt) - (1 << (cnt - 1))


class Font:
    """Minimal u8g2 font: header fields, glyph lookup and per-glyph metrics."""

    def __init__(self, blob: bytes):
        self.b = blob
        self.bpcw, self.bpch = blob[4], blob[5]
        self.bpcx, self.bpcy, self.bpdx = blob[6], blob[7], blob[8]
        self.bits_per_0, self.bits_per_1 = blob[2], blob[3]
        self.upper = (blob[17] << 8) | blob[18]
        self.lower = (blob[19] << 8) | blob[20]

    def glyph(self, enc: int):
        off = 23
        if enc >= ord("a"):
            off += self.lower
        elif enc >= ord("A"):
            off += self.upper
        p = off
        while self.b[p + 1] != 0:
            if self.b[p] == enc:
                return p + 2
            p += self.b[p + 1]
        return None

    def metrics(self, enc: int):
        g = self.glyph(enc)
        if g is None:
            return None
        r = _Bits(self.b[g:])
        return {"w": r.u(self.bpcw), "h": r.u(self.bpch), "ox": r.s(self.bpcx),
                "oy": r.s(self.bpcy), "dx": r.s(self.bpdx)}

    def width(self, text: str) -> int:
        """u8g2_GetStrWidth."""
        w = dx = gw = ox = 0
        for ch in text:
            m = self.metrics(ord(ch))
            if m is None:
                continue
            dx, gw, ox = m["dx"], m["w"], m["ox"]
            w += dx
        if gw != 0:
            w = w - dx + gw + ox
        return w


FONTS = {name: Font(_extract_font("u8g2_font_%s_tr" % name))
         for name in ("profont12", "profont15", "profont17", "profont22", "profont29")}


# ------------------------------------------------------------------------- canvas
class Canvas:
    """128x64 mono framebuffer with u8g2 semantics: 0 = clear, 1 = set, 2 = XOR."""

    def __init__(self):
        self.px = [[0] * W for _ in range(H)]
        self.color = 1
        self.font: Font = FONTS["profont12"]
        self.dir = 0

    # -- primitives ---------------------------------------------------------
    def set_color(self, color: int) -> None:
        self.color = color

    def set_font(self, name: str) -> None:
        self.font = FONTS[name]

    def set_font_direction(self, direction: int) -> None:
        self.dir = direction

    def _pix(self, x: int, y: int) -> None:
        if 0 <= x < W and 0 <= y < H:
            if self.color == 0:
                self.px[y][x] = 0
            elif self.color == 1:
                self.px[y][x] = 1
            else:
                self.px[y][x] ^= 1

    def _hvline(self, x: int, y: int, length: int, vertical: bool) -> None:
        for i in range(length):
            if vertical:
                self._pix(x, y + i)
            else:
                self._pix(x + i, y)

    def draw_box(self, x: int, y: int, w: int, h: int) -> None:
        for row in range(h):
            self._hvline(x, y + row, w, False)

    def draw_frame(self, x: int, y: int, w: int, h: int) -> None:
        """Same edge arithmetic as u8g2_DrawFrame."""
        xtmp = x
        self._hvline(x, y, w, False)
        if h >= 2:
            h -= 2
            y += 1
            if h > 0:
                self._hvline(x, y, h, True)
                x += w - 1
                self._hvline(x, y, h, True)
                y += h
            self._hvline(xtmp, y, w, False)

    # -- text ---------------------------------------------------------------
    def _vector_x(self, dx: int, x: int, y: int) -> int:
        return {0: dx + x, 1: dx - y, 2: dx - x}.get(self.dir, dx + y)

    def _vector_y(self, dy: int, x: int, y: int) -> int:
        return {0: dy + y, 1: dy + x, 2: dy - y}.get(self.dir, dy - x)

    def draw_str(self, x: int, y: int, text: str) -> None:
        """u8g2_DrawStr in transparent font mode, honouring the font direction."""
        for ch in text:
            m = self.font.metrics(ord(ch))
            if m is None:
                continue
            gw, gh = m["w"], m["h"]
            tx = self._vector_x(x, m["ox"], -(gh + m["oy"]))
            ty = self._vector_y(y, m["ox"], -(gh + m["oy"]))
            if gw > 0:
                self._glyph_pixels(tx, ty, ord(ch), gw, gh)
            if self.dir == 3:
                y -= m["dx"]
            else:
                x += m["dx"]

    def _glyph_pixels(self, tx: int, ty: int, enc: int, gw: int, gh: int) -> None:
        """Decode one glyph's RLE runs and plot the foreground pixels."""
        r = _Bits(self.font.b[self.font.glyph(enc):])
        r.u(self.font.bpcw)
        r.u(self.font.bpch)
        r.s(self.font.bpcx)
        r.s(self.font.bpcy)
        r.s(self.font.bpdx)
        lx = ly = 0
        while True:
            a = r.u(self.font.bits_per_0)
            b = r.u(self.font.bits_per_1)
            while True:
                for run, foreground in ((a, False), (b, True)):
                    cnt, cx, cy = run, lx, ly
                    while True:
                        rem = gw - cx
                        cur = cnt if cnt < rem else rem
                        if foreground:
                            for i in range(cur):
                                self._pix(self._vector_x(tx, cx + i, cy),
                                          self._vector_y(ty, cx + i, cy))
                        if cnt < rem:
                            break
                        cnt -= rem
                        cx, cy = 0, cy + 1
                    lx, ly = cx + cnt, cy
                if r.u(1) == 0:
                    break
            if ly >= gh:
                break

    def int_pad(self, x: int, y: int, num: int, digits: int) -> None:
        """u8g2_DrawIntZeroPad."""
        self.draw_str(x, y, "%0*d" % (digits, num))


# ---------------------------------------------------------------- the faces
def draw_clock_date(c: Canvas, t, focused=False, field=None, editing=False):
    """Core/Src/display_draw.c: draw_clock_date()."""
    c.set_font("profont17")
    c.int_pad(15, 52, t["day"], 2)
    c.draw_str(35, 52, "/")
    c.int_pad(47, 52, t["month"], 2)
    c.draw_str(67, 52, "/")
    c.int_pad(79, 52, t["year"], 4)
    c.int_pad(97, 31, t["seconds"], 2)

    c.set_font("profont29")
    c.int_pad(15, 31, t["hours"], 2)
    c.int_pad(62, 31, t["minutes"], 2)
    c.draw_str(47, 31, ":" if t["seconds"] % 2 == 0 else " ")

    c.set_color(2)
    c.draw_box(12, 9, 105, 25)

    if focused:
        boxes = {"hours": (13, 10, 34, 23), "minutes": (60, 10, 34, 23),
                 "seconds": (95, 18, 21, 15), "day": (13, 39, 21, 15),
                 "month": (45, 39, 21, 15), "year": (77, 39, 39, 15)}
        x, y, w, h = boxes[field]
        c.draw_frame(x, y, w, h)
        if editing:
            c.draw_box(x, y, w, h)

    c.set_color(1)


def draw_stop_watch(c: Canvas, elapsed_ms, running, menu, focused):
    """Core/Src/display_draw.c: draw_stop_watch()."""
    total = elapsed_ms // 1000
    hours, minutes, seconds = total // 3600, (total // 60) % 60, total % 60

    c.set_font("profont29")
    c.int_pad(8, 42, hours, 2)
    c.draw_str(39, 42, ":")
    c.int_pad(53, 42, minutes, 2)
    c.draw_str(84, 42, ":")

    c.set_font("profont22")
    c.int_pad(98, 42, seconds, 2)

    c.set_font("profont12")
    c.draw_str(38, 12, "STOPWATCH")
    c.draw_frame(36, 2, 57, 12)

    c.set_font("profont15")
    c.draw_str(22, 59, "RESET")
    c.draw_str(71, 59, "PAUSE" if running else "START")

    c.draw_frame(20, 48, 39, 13)
    c.draw_frame(69, 48, 39, 13)

    if focused:
        c.set_color(2)
        c.draw_box(20, 48, 39, 13) if menu == "reset" else c.draw_box(69, 48, 39, 13)
        c.draw_box(36, 2, 57, 12)
        c.set_color(1)


def draw_timer(c: Canvas, remaining_ms, state, field, editing, focused):
    """Core/Src/display_draw.c: draw_timer()."""
    total = remaining_ms // 1000
    minutes, seconds = total // 60, total % 60

    c.set_font("profont12")
    c.draw_str(50, 12, "TIMER")
    c.draw_frame(48, 2, 33, 12)

    c.set_font("profont29")
    c.draw_str(58, 39, ":")
    c.int_pad(28, 40, minutes, 2)
    c.int_pad(71, 40, seconds, 2)

    c.set_font("profont15")
    c.draw_str(22, 59, "RESET")
    c.draw_str(71, 59, "PAUSE" if state == "running" else "START")

    c.draw_frame(20, 48, 39, 13)
    c.draw_frame(69, 48, 39, 13)

    if focused:
        c.set_color(2)
        c.draw_box(48, 2, 33, 12)
        if state == "idle":
            if field == "mm":
                c.draw_frame(26, 19, 34, 23)
            elif field == "ss":
                c.draw_frame(69, 19, 34, 23)
        if field == "mm" and editing:
            c.draw_box(26, 19, 34, 23)
        elif field == "ss" and editing:
            c.draw_box(69, 19, 34, 23)
        elif field == "reset":
            c.draw_box(20, 48, 39, 13)
        elif field == "start_pause":
            c.draw_box(69, 48, 39, 13)
        c.set_color(1)

    if state == "paused":
        c.set_font("profont15")
        c.set_font_direction(3)
        c.draw_str(10, 43, "PAUSED")
        c.set_font_direction(0)


def draw_battery_percentage(c: Canvas, percentage, millivolts):
    """Core/Src/display_draw.c: draw_battery_percentage()."""
    percentage = min(percentage, 100)

    c.set_font("profont12")
    c.draw_str(44, 12, "BATTERY")
    c.draw_frame(42, 2, 45, 12)

    c.draw_frame(34, 17, 60, 31)     # body
    c.draw_frame(29, 28, 5, 9)       # terminal

    text = "%u%%" % percentage
    c.set_font("profont22")
    c.draw_str(35 + (58 - c.font.width(text)) // 2, 39, text)

    text = "%umV" % millivolts
    c.set_font("profont12")
    c.draw_str(64 - c.font.width(text) // 2, 59, text)

    charge_width = (58 * percentage) // 100
    if charge_width > 0:
        c.set_color(2)
        c.draw_box(35, 18, charge_width, 29)
        c.set_color(1)


def draw_face(state) -> Canvas:
    """Core/Src/display_draw.c: draw_face()."""
    c = Canvas()
    face = state["face"]
    if face == "clock":
        draw_clock_date(c, state["time"], state.get("focused", False),
                        state.get("field"), state.get("editing", False))
    elif face == "stopwatch":
        draw_stop_watch(c, state["elapsed_ms"], state["running"],
                        state.get("menu", "start_pause"), state.get("focused", False))
    elif face == "timer":
        draw_timer(c, state["remaining_ms"], state["state"], state.get("field", "mm"),
                   state.get("editing", False), state.get("focused", False))
    elif face == "battery":
        draw_battery_percentage(c, state["percentage"], state["millivolts"])

    if state.get("alarm") and state.get("flash", True):
        c.set_color(2)
        c.draw_box(0, 0, W, H)
        c.set_color(1)
    return c


# --------------------------------------------------------------------------- png
def write_png(path: str, canvases, gap: int = 4, scale: int = SCALE) -> None:
    """Write one or more canvases side by side (black background, white pixels)."""
    width = W * len(canvases) + gap * (len(canvases) - 1)
    rows = []
    for y in range(H * scale):
        row = bytearray([0])
        for idx, canvas in enumerate(canvases):
            for x in range(W * scale):
                on = canvas.px[y // scale][x // scale]
                row += b"\xff\xff\xff" if on else b"\x00\x00\x00"
            if idx != len(canvases) - 1:
                row += b"\x00\x00\x00" * (gap * scale)
        rows.append(bytes(row))
    raw = b"".join(rows)

    def chunk(tag, data):
        return (struct.pack(">I", len(data)) + tag + data
                + struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF))

    png = b"\x89PNG\r\n\x1a\n"
    png += chunk(b"IHDR", struct.pack(">IIBBBBB", width * scale, H * scale, 8, 2, 0, 0, 0))
    png += chunk(b"IDAT", zlib.compress(raw, 9))
    png += chunk(b"IEND", b"")
    with open(path, "wb") as f:
        f.write(png)


# ------------------------------------------------------------------------- gallery
TIME = {"hours": 13, "minutes": 45, "seconds": 42, "day": 27, "month": 8, "year": 2026}

GALLERY = [
    ("clock-idle", "Clock face, not focused",
     {"face": "clock", "time": TIME}),
    ("clock-select", "Clock focused, hours selected (hollow box)",
     {"face": "clock", "time": TIME, "focused": True, "field": "hours"}),
    ("clock-edit", "Clock editing the minutes (filled box)",
     {"face": "clock", "time": TIME, "focused": True, "field": "minutes", "editing": True}),
    ("clock-edit-date", "Clock editing the year (filled box on the date line)",
     {"face": "clock", "time": TIME, "focused": True, "field": "year", "editing": True}),
    ("stopwatch-idle", "Stopwatch, not focused, counting stopped",
     {"face": "stopwatch", "elapsed_ms": 0, "running": False}),
    ("stopwatch-running", "Stopwatch running (1:23:45), not focused",
     {"face": "stopwatch", "elapsed_ms": (1 * 3600 + 23 * 60 + 45) * 1000, "running": True}),
    ("stopwatch-focused", "Stopwatch focused, START/PAUSE box selected",
     {"face": "stopwatch", "elapsed_ms": (1 * 3600 + 23 * 60 + 45) * 1000,
      "running": True, "focused": True, "menu": "start_pause"}),
    ("timer-idle-select", "Timer idle and focused, minutes selected (hollow box)",
     {"face": "timer", "remaining_ms": 5 * 60 * 1000, "state": "idle",
      "focused": True, "field": "mm"}),
    ("timer-edit", "Timer editing the seconds (filled box)",
     {"face": "timer", "remaining_ms": 5 * 60 * 1000, "state": "idle",
      "focused": True, "field": "ss", "editing": True}),
    ("timer-running", "Timer running, not focused",
     {"face": "timer", "remaining_ms": 2 * 60 * 1000 + 7 * 1000, "state": "running"}),
    ("timer-paused", "Timer paused (rotated PAUSED label)",
     {"face": "timer", "remaining_ms": 2 * 60 * 1000 + 7 * 1000, "state": "paused"}),
    ("timer-alarm", "Timer alarm: whole screen XORed on the buzzer beat",
     {"face": "timer", "remaining_ms": 0, "state": "done", "alarm": True}),
    ("battery", "Battery face, 78 % at 4000 mV",
     {"face": "battery", "percentage": 78, "millivolts": 4000}),
    ("battery-low", "Battery face, 7 % at 3400 mV",
     {"face": "battery", "percentage": 7, "millivolts": 3400}),
]

OVERVIEW = ["clock-idle", "stopwatch-running", "timer-idle-select", "battery"]


def main() -> None:
    os.makedirs(IMAGE_DIR, exist_ok=True)
    canvases = {}
    for name, title, state in GALLERY:
        canvas = draw_face(state)
        canvases[name] = canvas
        write_png(os.path.join(IMAGE_DIR, name + ".png"), [canvas])
        print("%-18s %s" % (name + ".png", title))
    write_png(os.path.join(IMAGE_DIR, "overview.png"),
              [canvases[n] for n in OVERVIEW], gap=6)
    print("overview.png       the four faces side by side")


if __name__ == "__main__":
    main()
