"""Map/input regression: python tests/test_world.py (requires z80 and sjasmplus)."""

from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest

import z80


ROOT = Path(__file__).resolve().parents[1]


class World:
    def __init__(self, image, symbols, x=5, y=13, interrupt_after=None):
        self.s = symbols
        self.m = m = z80.Z80Machine()
        self.out = []
        m.set_memory_block(0x2000, image)
        self.put(chunk_x=x//15, chunk_y=y//4, player_x=8*(x%15), player_y=8*(y%4),
                 music_driver_installed=1, encounter_cooldown=5)
        for name in ("ROM_LCD_CLS", "ROM_T6834_CMD", "ROM_T6834_CMD_PARAM", "ROM_LCD_PUTC",
                     "ROM_MSG_DISPLAY_NOSTR", "ROM_HL_TO_ASCII_PRINT", "ROM_BREAKFLAG_QUERY"):
            m.memory[symbols[name]] = 0xC9  # Replace firmware calls, not game logic.
        m.set_memory_block(0x1E99, bytes.fromhex("ff a5 55 7f"))  # Former redraw state.
        m.set_memory_block(symbols["saved_serial_vector"], bytes.fromhex("c3 a3 c7"))
        for name, value in (("music_ptr", symbols["map_music"]), ("music_loop_ptr", symbols["map_music"]),
                            ("saved_return_sp", 0x1800)):
            m.set_memory_block(symbols[name], value.to_bytes(2, "little"))
        m.sp = 0x1800
        m.set_memory_block(m.sp, (0x1700).to_bytes(2, "little"))
        m.pc = symbols["main"]
        reads = 0

        def read_port(port):
            nonlocal reads
            if port & 255 != 0xF2:
                return 0
            reads += 1
            return (2 if reads % 7 else 0) | (4 if reads % 11 == 0 else 0)

        def write_port(port, value):
            if port & 255 == 0xF1:
                self.out.append(value)
                if interrupt_after is not None and len(self.out) == interrupt_after * 4:
                    self.put(SYS_BREAKFLAG=1)

        m.set_input_callback(read_port)
        m.set_output_callback(write_port)

    def put(self, **values):
        for name, value in values.items():
            self.m.memory[self.s[name]] = value

    def get(self, name):
        return self.m.memory[self.s[name]]

    def until(self, label):
        addr = self.s[label] if isinstance(label, str) else label
        self.m.set_breakpoint(addr)
        for _ in range(150):
            self.m.run()
            if self.m.pc == addr:
                self.m.clear_breakpoint(addr)
                return
        raise AssertionError(f"Never reached {label}; PC={self.m.pc:#06x}")

    def key(self, cmd, reply):
        self.until("ROM_T6834_CMD_PARAM")
        assert self.m.a == self.s[cmd] and self.m.de == self.s["SYS_T6834_REPLYBUF"]
        self.put(SYS_T6834_REPLYBUF=reply)

    def arrow(self, direction):
        self.key("T6834_CMD_TEST_KEY", 0)
        self.key("T6834_STICK_CMD", self.s["T6834_STICK_" + direction])

    def frame(self, x, y):
        ram = self.m.memory
        expected = []
        for line in range(32):
            for col in range(15):
                wx, wy = x//15*15 + col, y//4*4 + line//8
                packed = ram[self.s["map_packed"] + 15*wy + wx//2]
                tile = packed & 15 if wx % 2 else packed >> 4
                addr = 0xE000 + line*16 + col
                expected.extend((6, ram[self.s["tiles"] + tile*8 + line%8], addr & 255, addr >> 8))
        for line in range(8):
            addr = 0xE000 + (y%4*8 + line)*16 + x%15
            expected.extend((6, ram[self.s["player_sprite"] + line], addr & 255, addr >> 8))
        return expected


class WorldTest(unittest.TestCase):
    def test_release_builds(self):
        asm = shutil.which("sjasmplus") or ROOT.parent / "sjasmplus.exe"
        with tempfile.TemporaryDirectory() as tmp:
            binary, sym = Path(tmp)/"quest.bin", Path(tmp)/"quest.sym"
            for lang in (0, 1):
                with self.subTest(english=lang):
                    subprocess.run([asm, f"-DBUILD_LANG_EN={lang}", f"--raw={binary}", f"--sym={sym}",
                                    "quest.z80"], cwd=ROOT / "src", check=True, capture_output=True)
                    s = {n: int(v, 16) for n, v in re.findall(r"(?m)^(\w+): EQU 0x(\w+)", sym.read_text())}
                    image = binary.read_bytes()
                    self.assertLessEqual(0x2000 + len(image), 0x3F00)
                    for cx in range(2):
                        for cy in range(4):
                            x, y = cx*15+5, cy*4+1
                            g = World(image, s, x, y)
                            g.m.pc = s["world_redraw_map_with_player"]
                            g.until(0x1700)
                            self.assertEqual(g.out, g.frame(x, y))
                            self.assertEqual(g.get("music_render_active"), 0)

                    g = World(image, s)
                    for key in (2, 0):
                        g.key("T6834_CMD_TEST_KEY", key)
                    g.until("world_show_status_menu_wait")
                    for key in (2, 0):
                        g.key("T6834_CMD_TEST_KEY", key)
                    g.until("main")
                    self.assertEqual(g.out, g.frame(5, 13))
                    self.assertEqual(g.m.sp, 0x1800)

                    for x, y, direction, xx, yy in ((14,13,"RIGHT",15,13), (15,13,"LEFT",14,13),
                                                    (5,11,"DOWN",5,12), (5,12,"UP",5,11),
                                                    (5,13,"RIGHT",6,13)):
                        g = World(image, s, x, y)
                        g.arrow(direction)
                        g.until("check_tile_event")
                        g.until("check_random_battle")
                        g.until("main")
                        self.assertEqual([g.get(k) for k in ("save_chunk_x","save_chunk_y","player_x","player_y")],
                                         [xx//15, yy//4, 8*(xx%15), 8*(yy%4)])
                        if (x//15,y//4) != (xx//15,yy//4):
                            self.assertEqual(g.out, g.frame(xx, yy))
                        else:
                            self.assertEqual(len(g.out), 16*4)
                        self.assertEqual(g.m.sp, 0x1800)

                    g = World(image, s, 8, 13)
                    g.arrow("RIGHT")
                    g.until("main")
                    self.assertFalse(g.out)
                    self.assertEqual(g.get("player_x"), 64)
                    self.assertEqual(g.get("encounter_cooldown"), 5)
                    g = World(image, s, 2, 13)
                    g.arrow("RIGHT")
                    g.until("castle_menu_enter")
                    self.assertEqual(g.get("encounter_cooldown"), 5)

                    for cut in (1, 240, 479, 480, 481, 488):
                        g = World(image, s, interrupt_after=cut)
                        # Simulate the startup CALL: normal return enters main.
                        g.m.sp -= 2
                        g.m.set_memory_block(g.m.sp, s["main"].to_bytes(2, "little"))
                        g.m.pc = s["world_redraw_map_with_player"]
                        g.until(0x1700)
                        self.assertEqual(len(g.out), cut*4)
                        self.assertEqual(g.m.sp, 0x1802)
                        self.assertEqual(g.get("music_render_active"), 0)


if __name__ == "__main__":
    unittest.main()
