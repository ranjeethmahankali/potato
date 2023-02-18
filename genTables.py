"""Generate the C++ lookup tables."""
from datetime import datetime

BitBoard = "BitBoard"

# These only need to be enabled while debugging.
ENABLE_COMMENTS = False


class Board:
    """Represents a bit board."""

    def __init__(self):
        """Create a new empty board."""
        self.bits = [0 for _ in range(64)]
        self.descStr = ""

    def index(self, x, y):
        """Get index from 2d coords."""
        return x + y * 8

    def set(self, x, y):
        """Set a bit to 1."""
        self.bits[self.index(x, y)] = 1

    def unset(self, x, y):
        """Set a bit to 0."""
        self.bits[self.index(x, y)] = 0

    def bin(self):
        """Get a binary string."""
        return "0b" + "".join(reversed([str(b) for b in self.bits]))

    def hex(self):
        """Get the hexadecimal string."""
        return hex(int("".join(reversed([str(b) for b in self.bits])), 2))

    def flip(self):
        """Flip all the bits."""
        for i in range(len(self.bits)):
            self.bits[i] = 1 if self.bits[i] == 0 else 0

    def setdesc(self, txt):
        """Set metadata string."""
        self.descStr = txt

    def desc(self):
        """Get metadata string."""
        return self.descStr

    def __str__(self):
        """Get the string representation for printing."""
        txt = "\n"
        for y in range(8):
            txt += "|"
            for x in range(8):
                txt += 'X|' if self.bits[self.index(x, y)] == 1 else '_|'
            if y < 7:
                txt += '\n'
        return txt


def printArrayElems(boards):
    """Print the elements of a board array."""
    comments = [
        f'/* {i}: {b.desc()}{str(b)}*/\n'
        for b, i in zip(boards, range(len(boards)))
    ]
    empty = ''
    print(",\n".join([
        f'{comments[i] if ENABLE_COMMENTS else empty}{b.hex()}'
        for b, i in zip(boards, range(len(boards)))
    ]))


def printBoardArray(boards, name):
    """Print the boards as a constexpr array."""
    print(
        f"static constexpr std::array<{BitBoard}, {len(boards)}> {name} = {{{{"
    )
    printArrayElems(boards)
    print("}};")


def print2dBoardArray(boards, name):
    """Print a 2d lookup table of boards."""
    len1 = len(boards)
    len2 = len(boards[0])
    print(
        f"static constexpr std::array<std::array<{BitBoard}, {len2}>, {len1}>"
        f" {name} = {{{{\n")
    for nested in boards:
        print("{{")
        assert len(nested) == len2
        printArrayElems(nested)
        print("}},\n")
    print("}};")


def isOnBoard(x, y):
    """Check if a square is on the board."""
    return (x > -1 and x < 8 and y > -1 and y < 8)


def segment(xbegin,
            ybegin,
            xinc,
            yinc,
            xend,
            yend,
            withStart=True,
            withEnd=False):
    """Get a board with squares in a orthogonal or diagonal segment."""
    b = Board()
    x = xbegin
    y = ybegin
    while (x != xend or y != yend) and isOnBoard(x, y):
        b.set(x, y)
        x += xinc
        y += yinc
    if withEnd:
        b.set(xend, yend)
    if not withStart:
        b.unset(xbegin, ybegin)
    b.setdesc(f'Segment between ({xbegin}, {ybegin}) and ({xend}, {yend})')
    return b


def fileMask(x, y):
    """Get a board with the squares in a file."""
    return segment(x, 0, 0, 1, x, 8)


def rankMask(x, y):
    """Get a board with the squares in a rank."""
    return segment(0, y, 1, 0, 8, y)


def diagonalMask(x, y):
    """Get a board with all squared of the diagonal containing this square."""
    if y > x:
        return segment(0, y - x, 1, 1, 7 - y + x, 7, True, True)
    else:
        return segment(x - y, 0, 1, 1, 7, 7 - x + y, True, True)


def antiDiagonalMask(x, y):
    """Get a board with all squares of the anti-diagonal with this square."""
    if x + y < 8:
        return segment(0, x + y, 1, -1, x + y, 0, True, True)
    else:
        return segment(x + y - 7, 7, 1, -1, 7, x + y - 7, True, True)


def knightMovesMask(x, y):
    """Get positions attacked by a knight."""
    targets = [
        (x + s[0], y + s[1])
        for s in [(1, 2), (2, 1), (-1, 2), (-2,
                                            1), (1, -2), (2, -1), (-1,
                                                                   -2), (-2,
                                                                         -1)]
    ]
    b = Board()
    for i, j in targets:
        if isOnBoard(i, j):
            b.set(i, j)
    return b


def kingMovesMask(x, y):
    """Get positions attacked by the king."""
    targets = [
        (x + i, y + j)
        for i, j in [(1, 0), (-1,
                              0), (0, 1), (0, -1), (1, 1), (1,
                                                            -1), (-1,
                                                                  -1), (-1, 1)]
    ]
    b = Board()
    for i, j in targets:
        if isOnBoard(i, j):
            b.set(i, j)
    return b


def pawnCapturesMask(x, y, isWhite=False):
    """Get a mask for pawn captures from a square."""
    forward = -1 if isWhite else 1
    b = Board()
    if y == 7:
        return b
    if x > 0:
        b.set(x - 1, y + forward)
    if x < 7:
        b.set(x + 1, y + forward)
    return b


def oneHot(x, y):
    """Get a board with a single bit set."""
    b = Board()
    b.set(x, y)
    return b


def between(x1, y1, x2, y2):
    """Get a board with squares between the two squares."""
    if x1 == x2:
        return segment(x1,
                       y1,
                       0,
                       1 if y2 > y1 else -1,
                       x2,
                       y2,
                       withStart=False)
    elif y1 == y2:
        return segment(x1,
                       y1,
                       1 if x2 > x1 else -1,
                       0,
                       x2,
                       y2,
                       withStart=False)
    elif y1 - x1 == y2 - x2:
        return segment(x1, y1, 1, 1, x2, y2, withStart=False)
    elif x1 + y1 == x2 + y2:
        return segment(x1,
                       y1,
                       1 if x2 > x1 else -1,
                       1 if y2 > y1 else -1,
                       x2,
                       y2,
                       withStart=False)
    else:
        b = Board()
        b.setdesc(f'Segment between ({x1}, {y1}) and ({x2}, {y2})')
        return b


def table(mapfn):
    """Get a table mapping positions to tables."""
    boards = []
    for y in range(8):
        for x in range(8):
            boards.append(mapfn(x, y))
    return boards


def table2d(mapfn):
    """Generate a 2d table to maps for masks for every pair of squares."""
    tbl = []
    for y1 in range(8):
        for x1 in range(8):
            tbl.append(table(lambda x, y: mapfn(x1, y1, x, y)))
            assert len(tbl[-1]) == 64
    return tbl


if __name__ == "__main__":
    print("/*\nThis file is auto generated.\n"
          "DO NOT EDIT or track this file with git.\nGenerated at: "
          f"{datetime.now()}.\n*/\n")
    print('#pragma once\n')
    print('#include <array>\n')
    print('#include <stdint.h>\n')
    print('namespace potato {\n')
    print('using BitBoard = uint64_t;\n')
    printBoardArray(table(oneHot), "OneHot")
    print("")
    printBoardArray(table(fileMask), "File")
    print("")
    printBoardArray(table(rankMask), "Rank")
    print("")
    printBoardArray(table(diagonalMask), "Diagonal")
    print("")
    printBoardArray(table(antiDiagonalMask), "AntiDiagonal")
    print("")
    print2dBoardArray(table2d(between), "Between")
    print("")
    printBoardArray(table(knightMovesMask), "KnightMoves")
    print("")
    printBoardArray(table(kingMovesMask), "KingMoves")
    print("")
    printBoardArray(table(lambda x, y: pawnCapturesMask(x, y, False)),
                    "BlackPawnCaptures")
    print("")
    printBoardArray(table(lambda x, y: pawnCapturesMask(x, y, True)),
                    "WhitePawnCaptures")
    print('\n} // namespace potato')
