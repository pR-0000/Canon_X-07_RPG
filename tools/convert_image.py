#!/usr/bin/env python3
"""
Convertit un BMP 1-bit en directives Z80 ASM.

Format de sortie par defaut:
- une ligne ".db %xxxxxxxx, %xxxxxxxx, ..." par coordonnee Y
- les octets horizontaux (coordonnee X) sont a la suite sur la meme ligne

Exemple pour une image 24 px de large:
    .db %........, %........, %........
"""

import argparse
import os
import struct
import sys


def read_u16_le(data: bytes, off: int) -> int:
    return struct.unpack_from("<H", data, off)[0]


def read_u32_le(data: bytes, off: int) -> int:
    return struct.unpack_from("<I", data, off)[0]


def read_i32_le(data: bytes, off: int) -> int:
    return struct.unpack_from("<i", data, off)[0]


def sanitize_label(name: str) -> str:
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


def reverse_bits_8(value: int) -> int:
    out = 0
    for _ in range(8):
        out = (out << 1) | (value & 1)
        value >>= 1
    return out


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Convertit un BMP 1-bit en .db %xxxxxxxx pour Z80 ASM."
    )
    parser.add_argument("input_bmp", help="Chemin du fichier .bmp (1-bit).")
    parser.add_argument(
        "-o",
        "--output",
        help="Fichier de sortie .asm/.txt (defaut: <input>.asm).",
    )
    parser.add_argument(
        "--label",
        help="Label ASM a utiliser (defaut: nom du fichier).",
    )
    parser.add_argument(
        "--invert",
        dest="invert",
        action="store_true",
        default=True,
        help="Inverse 0/1 (defaut: actif).",
    )
    parser.add_argument(
        "--no-invert",
        dest="invert",
        action="store_false",
        help="Desactive l'inversion noir/blanc.",
    )
    parser.add_argument(
        "--lsb-left",
        action="store_true",
        help="Inverse l'ordre des bits dans chaque octet (bit 0 = pixel le plus a gauche).",
    )
    args = parser.parse_args()

    in_path = args.input_bmp
    out_path = args.output or (os.path.splitext(in_path)[0] + ".asm")

    with open(in_path, "rb") as fh:
        data = fh.read()

    if len(data) < 14 or data[0:2] != b"BM":
        print("Erreur: ce fichier n'est pas un BMP valide.", file=sys.stderr)
        return 1

    pixel_offset = read_u32_le(data, 10)
    dib_size = read_u32_le(data, 14)
    if dib_size < 40:
        print(f"Erreur: DIB header trop petit ({dib_size}).", file=sys.stderr)
        return 1

    width = read_i32_le(data, 18)
    height = read_i32_le(data, 22)
    planes = read_u16_le(data, 26)
    bpp = read_u16_le(data, 28)
    comp = read_u32_le(data, 30)

    if planes != 1:
        print(f"Erreur: planes={planes}, attendu 1.", file=sys.stderr)
        return 1
    if bpp != 1:
        print(f"Erreur: image {bpp}-bit, attendu 1-bit.", file=sys.stderr)
        return 1
    if comp != 0:
        print(f"Erreur: compression={comp}, attendu BI_RGB (0).", file=sys.stderr)
        return 1
    if width <= 0:
        print(f"Erreur: largeur={width}px, attendue > 0.", file=sys.stderr)
        return 1

    top_down = False
    if height < 0:
        top_down = True
        height = -height

    row_stride = ((width + 31) // 32) * 4
    width_bytes = (width + 7) // 8

    if pixel_offset + row_stride * height > len(data):
        print("Erreur: donnees BMP tronquees ou incoherentes.", file=sys.stderr)
        return 1

    base = os.path.splitext(os.path.basename(in_path))[0]
    label = sanitize_label(args.label or base)

    lines = [
        f"{label}: ; ({base}) {width}x{height}px, {width_bytes} octet(s) par ligne"
    ]

    for y in range(height):
        file_row = y if top_down else (height - 1 - y)
        row_off = pixel_offset + file_row * row_stride
        asm_bytes = []

        for x_byte in range(width_bytes):
            value = data[row_off + x_byte]

            if args.invert:
                value ^= 0xFF

            if x_byte == width_bytes - 1 and (width & 7):
                valid_bits = width & 7
                mask = (0xFF << (8 - valid_bits)) & 0xFF
                value &= mask

            if args.lsb_left:
                value = reverse_bits_8(value)

            asm_bytes.append(f"%{value:08b}")

        lines.append("\t.db " + ", ".join(asm_bytes))

    with open(out_path, "w", encoding="utf-8", newline="\n") as fh:
        fh.write("\n".join(lines) + "\n")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
