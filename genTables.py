"""Generate the C++ lookup tables."""


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
        return "0b" + "".join([str(b) for b in self.bits])

    def hex(self):
        """Get the hexadecimal string."""
        return hex(int("".join([str(b) for b in self.bits]), 2))

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
            txt += '\n'
        return txt


def file(f):
    """Get a board with the squares in a file."""
    b = Board()
    for y in range(8):
        b.set(f, y)
    return b


def rank(r):
    """Get a board with the squares in a rank."""
    b = Board()
    for x in range(8):
        b.set(x, r)
    return b


def diagonal(x, y):
    """Get a board with all squared of the diagonal containing this square."""
    if y > x:
        y = y - x
        x = 0
    else:
        x = x - y
        y = 0
    b = Board()
    while x < 8 and y < 8:
        b.set(x, y)
        x += 1
        y += 1
    return b


def antiDiagonal(x, y):
    """Get a board with all squares of the anti-diagonal with this square."""
    if x + y < 8:
        y = x + y
        x = 0
    else:
        x = x + y - 7
        y = 7
    b = Board()
    while x < 8 and y > -1:
        b.set(x, y)
        x += 1
        y -= 1
    return b


if __name__ == "__main__":
    print(rank(4))
    print(file(4))
    print(diagonal(4, 2))
    print(diagonal(2, 4))
    print(antiDiagonal(6, 2))
    print(antiDiagonal(2, 2))
