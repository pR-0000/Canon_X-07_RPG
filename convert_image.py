#!/usr/bin/env python3
# bmp8toz80db.py
# Convertit un BMP 1-bit (largeur 8 px) en directives Z80 ASM: .db %xxxxxxxx par ligne

import argparse
import os
import struct
import sys

def read_u16_le(b: bytes, off: int) -> int:
    return struct.unpack_from("<H", b, off)[0]

def read_u32_le(b: bytes, off: int) -> int:
    return struct.unpack_from("<I", b, off)[0]

def read_i32_le(b: bytes, off: int) -> int:
    return struct.unpack_from("<i", b, off)[0]

def sanitize_label(name: str) -> str:
    # label ASM "simple": lettres/chiffres/_ , ne commence pas par chiffre
    out = []
    for ch in name:
        if ch.isalnum() or ch == "_":
            out.append(ch)
        else:
            out.append("_")
    label = "".join(out)
    if not label or label[0].isdigit():
        label = "_" + label
    return label

def main() -> int:
    ap = argparse.ArgumentParser(
        description="Convertit un BMP 1-bit de 8 px de large en .db %xxxxxxxx (Z80 ASM)."
    )
    ap.add_argument("input_bmp", help="Chemin du fichier .bmp (1-bit, largeur=8).")
    ap.add_argument("-o", "--output", help="Fichier de sortie .asm/.txt (défaut: <input>.asm).")
    ap.add_argument("--label", help="Label ASM à utiliser (défaut: nom de fichier).")
    ap.add_argument("--invert", action="store_true", default=True, help="Inverse 0/1 (utile si noir/blanc inversés).")
    ap.add_argument("--msb-left", action="store_true",
                    help="Bit 7 = pixel le plus à gauche (défaut BMP: bit 7 = gauche aussi pour 1bpp).")
    args = ap.parse_args()

    in_path = args.input_bmp
    out_path = args.output or (os.path.splitext(in_path)[0] + ".asm")

    with open(in_path, "rb") as f:
        data = f.read()

    # --- BITMAPFILEHEADER (14 octets) ---
    if len(data) < 14 or data[0:2] != b"BM":
        print("Erreur: ce fichier n'est pas un BMP (signature BM absente).", file=sys.stderr)
        return 1

    pixel_offset = read_u32_le(data, 10)

    # --- BITMAPINFOHEADER (au moins 40 octets) ---
    dib_size = read_u32_le(data, 14)
    if dib_size < 40:
        print(f"Erreur: DIB header trop petit ({dib_size}).", file=sys.stderr)
        return 1

    width  = read_i32_le(data, 18)
    height = read_i32_le(data, 22)
    planes = read_u16_le(data, 26)
    bpp    = read_u16_le(data, 28)
    comp   = read_u32_le(data, 30)

    if planes != 1:
        print(f"Erreur: planes={planes}, attendu 1.", file=sys.stderr)
        return 1
    if bpp != 1:
        print(f"Erreur: image {bpp}-bit, attendu 1-bit.", file=sys.stderr)
        return 1
    if comp != 0:
        print(f"Erreur: compression={comp}, attendu BI_RGB (0).", file=sys.stderr)
        return 1
    if width != 8:
        print(f"Erreur: largeur={width}px, attendu 8px.", file=sys.stderr)
        return 1

    top_down = False
    if height < 0:
        top_down = True
        height = -height

    stride = ((width + 31) // 32) * 4

    if pixel_offset + stride * height > len(data):
        print("Erreur: données BMP tronquées/incohérentes.", file=sys.stderr)
        return 1

    # Label
    base = os.path.splitext(os.path.basename(in_path))[0]
    label = sanitize_label(args.label or base)

    lines = []
    lines.append(f"{label}: ; ({base})")

    for y in range(height):
        file_row = y if top_down else (height - 1 - y)
        row_off = pixel_offset + file_row * stride
        byte0 = data[row_off]  # seul octet utile (8 pixels)
        b = byte0
        if args.invert:
            b ^= 0xFF

        bits = format(b, "08b")
        lines.append(f"\t.db %{bits}")

    with open(out_path, "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(lines) + "\n")

    return 0

if __name__ == "__main__":
    raise SystemExit(main())
