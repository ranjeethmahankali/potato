"""This script is a debugging tool.

This will help fix bugs in move generation by comparing the move
generation of potato with that of stockfish.
"""

import subprocess


class Root:

    def __init__(self, fen=None):
        self.fen = fen

    def perft(self):
        fish = subprocess.Popen('stockfish',
                                stdin=subprocess.PIPE,
                                stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE)
        out, err = fish.communicate(
            input=bytes("position startpos\ngo perft 4\n", "ascii"))
        fish.kill()
        lines = sorted([
            ln.lower() for ln in out.decode().splitlines()[1:]
            if ln and not ln.lower().startswith("nodes searched")
        ])
        pairs = [ln.split(": ") for ln in lines]
        return [(p[0], int(p[1])) for p in pairs]


r = Root()
r.perft()
