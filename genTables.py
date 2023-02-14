"""Generate the C++ lookup tables."""

BitBoard = "BitBoard"

# These only need to be enabled while debugging.
ENABLE_COMMENTS = False


class Board:
    """Represents a bit board."""

    def __init__(self):
        """Create a new empty board."""
        self.bits = [0 for _ in range(64)]

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


def printBoardArray(boards, name):
    """Print the boards as a constexpr array."""
    print(
        f"static constexpr std::array<{BitBoard}, {len(boards)}> {name} = {{{{"
    )
    comments = [
        f'/* {i}{str(b)}*/\n' for b, i in zip(boards, range(len(boards)))
    ]
    empty = ''
    print(",\n".join([
        f'{comments[i] if ENABLE_COMMENTS else empty}{b.hex()}'
        for b, i in zip(boards, range(len(boards)))
    ]))
    print("}}}};")


def isOnBoard(x, y):
    """Check if a square is on the board."""
    return (x > -1 and x < 8 and y > -1 and y < 8)


def segment(xbegin, ybegin, xinc, yinc, xend, yend, withEnd=False):
    """Get a board with squares in a orthogonal or diagonal segment."""
    b = Board()
    while (xbegin != xend or ybegin != yend) and isOnBoard(xbegin, ybegin):
        b.set(xbegin, ybegin)
        xbegin += xinc
        ybegin += yinc
    if withEnd:
        b.set(xend, yend)
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
        return segment(0, y - x, 1, 1, 7 - y + x, 7, True)
    else:
        return segment(x - y, 0, 1, 1, 7, 7 - x + y, True)


def antiDiagonalMask(x, y):
    """Get a board with all squares of the anti-diagonal with this square."""
    if x + y < 8:
        return segment(0, x + y, 1, -1, x + y, 0, True)
    else:
        return segment(x + y - 7, 7, 1, -1, 7, x + y - 7, True)


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


def table(mapfn):
    """Get a table mapping positions to tables."""
    boards = []
    for y in range(8):
        for x in range(8):
            boards.append(mapfn(x, y))
    return boards


if __name__ == "__main__":
    print('#pragma once\n')
    print('namespace potato {\n')
    printBoardArray(table(fileMask), "sFiles")
    print("")
    printBoardArray(table(rankMask), "sRanks")
    print("")
    printBoardArray(table(diagonalMask), "sDiagonals")
    print("")
    printBoardArray(table(antiDiagonalMask), "sAntiDiagonals")
    print("")
    printBoardArray(table(knightMovesMask), "sKnightMoves")
    print("")
    printBoardArray(table(kingMovesMask), "sKingMoves")
    print('\n} // namespace potato')
    # print(rank(4))
    # print(file(4))
    # print(diagonal(4, 2))
    # print(diagonal(2, 4))
    # print(antiDiagonal(6, 2))
    # print(antiDiagonal(2, 2))
