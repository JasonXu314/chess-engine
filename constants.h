#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "types.h"

enum Players { WHITE, BLACK };
enum Files { A, B, C, D, E, F, G, H };
enum PieceTypes { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING };

extern const Files FILES[8];
extern const uint RANKS[8];

uint valueOf(PieceTypes type);

#endif