"""This script is a debugging tool.

This will help fix bugs in move generation by comparing the move
generation of potato with that of stockfish.
"""

import subprocess


def makeNewFen(oldfen: str, move: str):
    """Get the new fen after applying moves to the old fen."""
    fish = subprocess.Popen('stockfish',
                            stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE)
    out, err = fish.communicate(input=bytes(
        f"position fen \"{oldfen}\" moves {move}\nd\nquit\n", "ascii"))
    fish.kill()
    fenline = [
        ln for ln in out.decode().splitlines() if ln.startswith("Fen: ")
    ][0]
    return fenline.removeprefix('Fen: ')


def fishMoveCount(fenstr: str, depth: int):
    """Get the move count from the stockfish's perft."""
    fish = subprocess.Popen('stockfish',
                            stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE)
    out, err = fish.communicate(input=bytes(
        f"position fen \"{fenstr}\"\ngo perft {depth}\nquit\n", "ascii"))
    fish.kill()
    return int([
        ln.lower() for ln in out.decode().splitlines()[1:]
        if ln.lower().startswith("nodes searched: ")
    ][0].removeprefix("nodes searched: "))


class Comparator:
    """Useful for comparing move generation."""

    def __init__(self, fen=None, depth=4, ht=0):
        """Create a new search tree."""
        self.fen = fen
        self.depth = depth
        self.ht = ht

    def indent(self):
        """Get indentation for printing messages based on the search depth."""
        return '    ' * self.ht

    def perftFish(self):
        """Do perft with stockfish and get sorted results."""
        fish = subprocess.Popen('stockfish',
                                stdin=subprocess.PIPE,
                                stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE)
        out, err = fish.communicate(input=bytes(
            f"position fen \"{self.fen}\"\ngo perft {self.depth}\nquit\n",
            "ascii"))
        fish.kill()
        lines = sorted([
            ln.lower() for ln in out.decode().splitlines()[1:] if ln
            and ': ' in ln and not ln.lower().startswith("nodes searched")
        ])
        pairs = [ln.split(": ") for ln in lines]
        return {p[0]: int(p[1]) for p in pairs}

    def perftPotato(self):
        """Do perft with potato and get sorted results."""
        potato = subprocess.Popen(['Release/potato', '--cli'],
                                  stdin=subprocess.PIPE,
                                  stdout=subprocess.PIPE,
                                  stderr=subprocess.PIPE)
        out, err = potato.communicate(input=bytes(
            f"fen \"{self.fen}\"\nperft {self.depth}\nquit\n", "ascii"))
        potato.kill()
        lines = sorted([
            ln.lower() for ln in out.decode().splitlines() if ln and ': ' in ln
            and not ln.startswith('[') and not ln.startswith('Total')
        ])
        pairs = [ln.split(": ") for ln in lines]
        if (len(pairs) != len(set([p[0] for p in pairs]))):
            print(f"{self.indent()}!!! Repeated move.")
        return {p[0]: int(p[1]) for p in pairs}

    def compare(self):
        """Compare the results of stockfish and potato."""
        print(f"{self.indent()}Comparing fen: {self.fen}")
        fdata = self.perftFish()
        pdata = self.perftPotato()
        problems = []
        success = True
        for move in fdata:
            if move not in pdata:
                success = False
                print(f"{self.indent()}Move {move} not generated by potato.")
        for move in pdata:
            if move not in fdata:
                success = False
                print(f"{self.indent()}Move {move} is illegal.")
        for move in fdata:
            if move in pdata and fdata[move] != pdata[move]:
                success = False
                print(f"{self.indent()}The perft move count for {move} "
                      f"is incorrect ({fdata[move]} != {pdata[move]})")
                problems.append(move)
        if self.depth == 1:
            return success
        # Recursive compare for all the problems
        for problem in problems:
            print(f"{self.indent()}Recursing the search with move {problem}")
            newfen = makeNewFen(self.fen, problem)
            if newfen == self.fen:
                continue
            r = Comparator(newfen, self.depth - 1, self.ht + 1)
            childSuccess = r.compare()
            if childSuccess:
                print(f"{self.indent()}Inconsistent position:")
                print(f"{self.indent()}Before: {self.fen}")
                print(f"{self.indent()}After: {newfen}")
                print(f"{self.indent()}Move: {problem}")
                success = False
        return success


def comparefen(fenstr, depth):
    """Compare the move generation from the given fen to the given depth."""
    r = Comparator(fenstr, depth)
    if r.compare():
        print("All checks passed!")


def generateUnitTests(fenstrs, depth, counterOffset=1):
    """Generate C++ unit test for testing positions."""
    for counter, fenstr in zip(
            range(counterOffset,
                  len(fenstrs) + counterOffset), fenstrs):
        print(f"TEST_CASE(\"Move generation test {counter}\", "
              f"\"[move-gen][case-{counter}]\"){{\n"
              f"doPerftTest(\"{fenstr}\", {depth}, {{{{" + ", ".join(
                  [str(fishMoveCount(fenstr, d))
                   for d in range(1, depth + 1)]) + "}});\n}\n")


if __name__ == "__main__":
    comparefen("3Q4/8/7p/n5N1/P1b1K3/q5p1/1P2k3/B7 w - - 0 1", 6)
