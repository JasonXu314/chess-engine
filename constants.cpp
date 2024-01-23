#include "constants.h"

const Files FILES[8] = {A, B, C, D, E, F, G, H};
const uint RANKS[8] = {1, 2, 3, 4, 5, 6, 7, 8};
const PieceTypes PIECE_TYPES[6] = {PieceTypes::PAWN, PieceTypes::KNIGHT, PieceTypes::BISHOP, PieceTypes::ROOK, PieceTypes::QUEEN, PieceTypes::KING};

uint valueOf(PieceTypes type) {
	switch (type) {
		case PieceTypes::PAWN:
			return 1;
		case PieceTypes::KNIGHT:
		case PieceTypes::BISHOP:
			return 3;
		case PieceTypes::ROOK:
			return 5;
		case PieceTypes::QUEEN:
			return 9;
		case PieceTypes::KING:
			return (uint)-1;  // As close to infinity as we can get
		default:
			return 0;
	}
}