
#include <Position.h>
#include <iostream>

namespace potato {
using ZobristTable = std::array<size_t, NUniquePieces * 64>;

static ZobristTable generateTable()
{
  std::array<size_t, NUniquePieces * 64> table;
  for (size_t pc = 0; pc < 18; ++pc) {
    for (size_t pos = 0; pos < 64; ++pos) {
      table[pc * 64 + pos] = size_t(std::rand()) | (size_t(std::rand()) << 32);
    }
  }
  return table;
}

static const ZobristTable& getTable()
{
  static const ZobristTable sTable = generateTable();
  return sTable;
}

void Position::calcHash()
{
  // Compute zobrish hash.
  // TODO: Incomplete;
  mHash = 0;
}

}  // namespace potato
