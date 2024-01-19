#include "chess.h"

using namespace std;

bool operator==(const Position& a, const Position& b) { return a.file == b.file && a.rank == b.rank; }

string to_string(const Position& pos) {
	string out;

	out += ('A' + pos.file);
	out += to_string(pos.rank);

	return out;
}

Piece::Piece(char symbol, Position position)
	: _position(position), _player(isupper(symbol) ? Players::WHITE : Players::BLACK), _symbol(symbol), _moved(false) {
	switch (symbol) {
		case 'p':
		case 'P':
			_type = PieceTypes::PAWN;
			break;
		case 'n':
		case 'N':
			_type = PieceTypes::KNIGHT;
			break;
		case 'b':
		case 'B':
			_type = PieceTypes::BISHOP;
			break;
		case 'r':
		case 'R':
			_type = PieceTypes::ROOK;
			break;
		case 'q':
		case 'Q':
			_type = PieceTypes::QUEEN;
			break;
		case 'k':
		case 'K':
			_type = PieceTypes::KING;
			break;
		default:
			throw runtime_error(string("Invalid piece type '") + symbol + "'");
	}
}

const Position& Piece::position() const { return _position; }
PieceTypes Piece::type() const { return _type; }
Players Piece::player() const { return _player; }
char Piece::symbol() const { return _symbol; }

ostream& operator<<(ostream& out, const Piece& piece) {
	string name;

	switch (piece._type) {
		case PieceTypes::PAWN:
			name = "Pawn";
			break;
		case PieceTypes::KNIGHT:
			name = "Knight";
			break;
		case PieceTypes::BISHOP:
			name = "Bishop";
			break;
		case PieceTypes::ROOK:
			name = "Rook";
			break;
		case PieceTypes::QUEEN:
			name = "Queen";
			break;
		case PieceTypes::KING:
			name = "King";
			break;
		default:
			name = "Err";
			break;
	}

	return out << name << " (" << (piece._player == Players::WHITE ? "White" : "Black") << "), " << to_string(piece._position);
}

Game::Game() : _turn(Players::WHITE), _firstMove(true) {
	for (const Files file : FILES) {
		_board[file][2] = 'P';
		_board[file][7] = 'p';

		_white.push_back(Piece('P', {.file = file, .rank = 2}));
		_black.push_back(Piece('p', {.file = file, .rank = 7}));
	}

	_board[Files::A][1] = _board[Files::H][1] = 'R';
	_white.push_back(Piece('R', {.file = Files::A, .rank = 1}));
	_white.push_back(Piece('R', {.file = Files::H, .rank = 1}));
	_board[Files::B][1] = _board[Files::G][1] = 'N';
	_white.push_back(Piece('N', {.file = Files::B, .rank = 1}));
	_white.push_back(Piece('N', {.file = Files::G, .rank = 1}));
	_board[Files::C][1] = _board[Files::F][1] = 'B';
	_white.push_back(Piece('B', {.file = Files::C, .rank = 1}));
	_white.push_back(Piece('B', {.file = Files::F, .rank = 1}));
	_board[Files::D][1] = 'Q';
	_white.push_back(Piece('Q', {.file = Files::D, .rank = 1}));
	_board[Files::E][1] = 'K';
	_white.push_back(Piece('K', {.file = Files::E, .rank = 1}));

	_board[Files::A][8] = _board[Files::H][8] = 'r';
	_black.push_back(Piece('r', {.file = Files::A, .rank = 8}));
	_black.push_back(Piece('r', {.file = Files::H, .rank = 8}));
	_board[Files::B][8] = _board[Files::G][8] = 'n';
	_black.push_back(Piece('n', {.file = Files::B, .rank = 8}));
	_black.push_back(Piece('n', {.file = Files::G, .rank = 8}));
	_board[Files::C][8] = _board[Files::F][8] = 'b';
	_black.push_back(Piece('b', {.file = Files::C, .rank = 8}));
	_black.push_back(Piece('b', {.file = Files::F, .rank = 8}));
	_board[Files::D][8] = 'q';
	_black.push_back(Piece('q', {.file = Files::D, .rank = 8}));
	_board[Files::E][8] = 'k';
	_black.push_back(Piece('k', {.file = Files::E, .rank = 8}));
}

bool Game::move(const Move& move) {
	Piece& piece = _getPieceRef(move.from);

	if (piece._player != _turn) {
		throw runtime_error("Moved piece does not belong to moving player.");
	}
	if (move.from == move.to) {
		throw runtime_error("Illegal move: no move.");
	}

	// TODO: not a todo, just a note that isCapture is actually whether the destination has a piece because of the en passant edge case
	bool isCapture;
	if ((isCapture = hasPiece(move.to))) {
		Piece destPiece = getPiece(move.to);

		if (destPiece._player == _turn) {
			throw runtime_error("Cannot move to a square occupied by same player's piece.");
		}
	}

	switch (piece._type) {
		case PieceTypes::PAWN:
			_validatePawnMove(move);
			break;
		case PieceTypes::KNIGHT:
			_validateKnightMove(move);
			break;
		case PieceTypes::BISHOP:
			_validateBishopMove(move);
			break;
		case PieceTypes::ROOK:
			_validateRookMove(move);
			break;
		case PieceTypes::QUEEN:
			try {
				_validateRookMove(move);
			} catch (...) {
				try {
					_validateBishopMove(move);
				} catch (...) {
					throw runtime_error("Illegal queen move from " + to_string(move.from) + " to " + to_string(move.to) + ".");
				}
			}
			break;
		case PieceTypes::KING:
			_validateKingMove(move);
			break;
	}

	char temp = _board[move.to.file][move.to.rank];
	_board[move.from.file][move.from.rank] = '\0';
	_board[move.to.file][move.to.rank] = piece._symbol;

	vector<Piece>&player = _turn == Players::WHITE ? _white : _black, &other = _turn == Players::WHITE ? _black : _white;
	Position kingPos;

	if (piece._type == PieceTypes::KING) {
		kingPos = move.to;
	} else {
		for (const Piece& piece : player) {
			if (piece._type == PieceTypes::KING) {
				kingPos = piece._position;
			}
		}
	}

	for (const Piece& opponentPiece : other) {
		if (opponentPiece._position == move.to) continue;  // TODO: consider en passant

		switch (opponentPiece._type) {
			case PieceTypes::PAWN:
				try {
					// TODO: no idea if en passant is an edge case here? (i think it shouldnt)
					_validatePawnMove({.from = opponentPiece._position, .to = kingPos});
					break;	// breaks out of switch block, triggering "trap"
				} catch (...) {
					continue;  // skips the "trap" at the bottom of each iteration
				}
			case PieceTypes::KNIGHT:
				try {
					_validateKnightMove({.from = opponentPiece._position, .to = kingPos});
					break;
				} catch (...) {
					continue;
				}
			case PieceTypes::BISHOP:
				try {
					_validateBishopMove({.from = opponentPiece._position, .to = kingPos});
					break;
				} catch (...) {
					continue;
				}
			case PieceTypes::ROOK:
				try {
					_validateRookMove({.from = opponentPiece._position, .to = kingPos});
					break;
				} catch (...) {
					continue;
				}
			case PieceTypes::QUEEN:
				try {
					_validateRookMove({.from = opponentPiece._position, .to = kingPos});
					break;
				} catch (...) {
					try {
						_validateBishopMove({.from = opponentPiece._position, .to = kingPos});
						break;
					} catch (...) {
						continue;
					}
				}
			case PieceTypes::KING:
				try {
					_validateKingMove({.from = opponentPiece._position, .to = kingPos});
					break;
				} catch (...) {
					continue;
				}
		}

		// "undo" move
		_board[move.from.file][move.from.rank] = piece._symbol;
		_board[move.to.file][move.to.rank] = temp;
		throw runtime_error("Illegal move: moving into check/moving while in check.");
	}

	if (isCapture) {
		uint capturedIdx = (uint)-1;
		for (uint i = 0; i < other.size(); i++) {
			if (other[i]._position == move.to) {
				capturedIdx = i;
			}
		}

		if (capturedIdx != (uint)-1) {
			other.erase(other.begin() + capturedIdx);
		} else {
			throw runtime_error("Shit done fucked up (capture logic)");
		}
	} else if (piece._type == PieceTypes::PAWN && move.to.file != move.from.file && !_hasPiece(move.to) &&
			   _hasPiece({.file = move.to.file, .rank = move.to.rank + (_turn == Players::WHITE ? -1 : 1)})) {
		uint capturedIdx = (uint)-1;
		for (uint i = 0; i < other.size(); i++) {
			if (other[i]._position == Position{.file = move.to.file, .rank = move.to.rank + (_turn == Players::WHITE ? -1 : 1)} &&
				other[i]._type == PieceTypes::PAWN) {
				capturedIdx = i;
			}
		}

		if (capturedIdx != (uint)-1) {
			other.erase(other.begin() + capturedIdx);
			_board[move.to.file][move.to.rank + (_turn == Players::WHITE ? -1 : 1)] = '\0';
		} else {
			throw runtime_error("Shit done fucked up (capture logic)");
		}
	}

	piece._position = move.to;
	if (piece._type == PieceTypes::KING && abs((int)move.to.file - (int)move.from.file) == 2) {
		// consider castling
		int castleDir = (int)move.to.file - (int)move.from.file < 0 ? -1 : 1;
		Position rookPos = {.file = castleDir == -1 ? Files::A : Files::H, .rank = _turn == Players::WHITE ? 1u : 8u};
		Piece& rook = _getPieceRef(rookPos);

		Position rookTo = move.to;
		rookTo.file = (Files)((int)rookTo.file + (castleDir == -1 ? 1 : -1));

		rook._position = rookTo;
		_board[rookPos.file][rookPos.rank] = '\0';
		_board[rookTo.file][rookTo.rank] = (_turn == Players::WHITE ? 'R' : 'r');
	}

	_prevMove = move;

	if (_firstMove) {
		_firstMove = false;
	}
	if (!piece._moved) {
		piece._moved = true;
	}

	if (piece._type == PieceTypes::PAWN && move.to.rank == (_turn == Players::WHITE ? 8 : 1)) {
		return true;
	} else {
		_turn = (_turn == Players::WHITE ? Players::BLACK : Players::WHITE);

		return false;
	}
}

void Game::promote(const Position& pos, PieceTypes to) {
	Piece& piece = _getPieceRef(pos);

	if (piece._player != _turn) {
		throw runtime_error("Moved piece does not belong to moving player.");
	}
	if (piece._type != PieceTypes::PAWN) {
		throw runtime_error("Promotion piece must be a pawn.");
	}
	if (pos.rank != (_turn == Players::WHITE ? 8 : 1)) {
		throw runtime_error("Promotion rank must be back rank of opposing player.");
	}
	if (to == PieceTypes::PAWN) {
		throw runtime_error("Cannot promote to pawn (though i have no idea why you'd even attempt to do this).");
	}
	if (to == PieceTypes::KING) {
		throw runtime_error("Cannot promote to king (nice try).");
	}

	// TODO: consider moving this to private promotion method on piece
	piece._type = to;
	switch (to) {
		case PieceTypes::KNIGHT:
			piece._symbol = _turn == Players::WHITE ? 'N' : 'n';
			break;
		case PieceTypes::BISHOP:
			piece._symbol = _turn == Players::WHITE ? 'B' : 'b';
			break;
		case PieceTypes::ROOK:
			piece._symbol = _turn == Players::WHITE ? 'R' : 'r';
			break;
		case PieceTypes::QUEEN:
			piece._symbol = _turn == Players::WHITE ? 'Q' : 'q';
			break;
		default:
			throw runtime_error("Shit done fucked up (promotion)");
	}
	piece._value = valueOf(piece._type);

	_board[pos.file][pos.rank] = piece._symbol;
	_turn = (_turn == Players::WHITE ? Players::BLACK : Players::WHITE);
}

Piece Game::getPiece(const Position& pos) const {
	char symbol = _board[pos.file][pos.rank];

	if (symbol == '\0') {
		throw runtime_error("No piece at position " + to_string(pos));
	}

	for (const Piece& piece : _white) {
		if (piece._position == pos) {
			return piece;
		}
	}

	for (const Piece& piece : _black) {
		if (piece._position == pos) {
			return piece;
		}
	}

	throw runtime_error("Shit done fucked up (getPiece).");
}

Piece& Game::_getPieceRef(const Position& pos) {
	char symbol = _board[pos.file][pos.rank];

	if (symbol == '\0') {
		throw runtime_error("No piece at position " + to_string(pos));
	}

	for (Piece& piece : _white) {
		if (piece._position == pos) {
			return piece;
		}
	}

	for (Piece& piece : _black) {
		if (piece._position == pos) {
			return piece;
		}
	}

	throw runtime_error("Shit done fucked up (getPieceRef).");
}

bool Game::_hasPiece(const Position& pos) const {
	for (const Piece& piece : _white) {
		if (piece._position == pos) {
			return true;
		}
	}

	for (const Piece& piece : _black) {
		if (piece._position == pos) {
			return true;
		}
	}

	return false;
}

bool Game::hasPiece(const Position& pos) const { return _board[pos.file][pos.rank] != '\0'; }

const Board& Game::board() const { return _board; }
Players Game::turn() const { return _turn; }

void Game::_validatePawnMove(const Move& move) const {
	Piece piece = getPiece(move.from);

	bool isCapture =
		hasPiece(move.to) || (!_firstMove && (hasPiece(_prevMove.to) && getPiece(_prevMove.to)._type == PieceTypes::PAWN &&
											  _prevMove.from.rank == (_turn == Players::WHITE ? 7 : 2) && _prevMove.to.file == move.to.file &&
											  _prevMove.to.rank == move.to.rank + (_turn == Players::WHITE ? -1 : 1)));	 // this ugly logic is en passant

	if (isCapture && (move.to.rank != (_turn == Players::WHITE ? move.from.rank + 1 : move.from.rank - 1) ||
					  (move.to.file != move.from.file + 1 &&
					   move.to.file != move.from.file - 1))) {	// shouldn't have to worry about overflow because it won't be equal anyway
		throw runtime_error("Illegal pawn move from " + to_string(move.from) + " to " + to_string(move.to) + ": captures must be diagonal.");
	} else if (!isCapture) {
		if (move.to.file != move.from.file) {
			throw runtime_error("Illegal pawn move from " + to_string(move.from) + " to " + to_string(move.to) + ": normal moves must be straight.");
		}

		if (piece._position.rank == (_turn == Players::WHITE ? 2 : 7)) {
			// splitting up cases to make logic clearer
			if (_turn == Players::WHITE && (move.to.rank < move.from.rank || move.to.rank > move.from.rank + 2)) {
				throw runtime_error("Illegal pawn move from " + to_string(move.from) + " to " + to_string(move.to) +
									": normal move from starting position must be to one of the 2 tiles directly forwards.");
			} else if (_turn == Players::BLACK && (move.to.rank > move.from.rank || move.to.rank < move.from.rank - 2)) {
				throw runtime_error("Illegal pawn move from " + to_string(move.from) + " to " + to_string(move.to) +
									": normal move from starting position must be to one of the 2 tiles directly forwards.");
			}
		} else {
			if (_turn == Players::WHITE && (move.to.rank < move.from.rank || move.to.rank > move.from.rank + 1)) {
				throw runtime_error("Illegal pawn move from " + to_string(move.from) + " to " + to_string(move.to) +
									": normal move not from starting position must be to tile directly forwards.");
			} else if (_turn == Players::BLACK && (move.to.rank > move.from.rank || move.to.rank < move.from.rank - 1)) {
				throw runtime_error("Illegal pawn move from " + to_string(move.from) + " to " + to_string(move.to) +
									": normal move not from starting position must be to tile directly forwards.");
			}
		}
	}
}

void Game::_validateKnightMove(const Move& move) const {
	int diffRank = abs((int)move.to.rank - (int)move.from.rank);

	if (diffRank > 2 || diffRank < 1) {
		throw runtime_error("Illegal knight move from " + to_string(move.from) + " to " + to_string(move.to) + ".");
	}

	int diffFile = abs((int)move.to.file - (int)move.from.file);

	if (diffFile > 2 || diffFile < 1) {
		throw runtime_error("Illegal knight move from " + to_string(move.from) + " to " + to_string(move.to) + ".");
	}

	if (diffRank == diffFile) {
		throw runtime_error("Illegal knight move from " + to_string(move.from) + " to " + to_string(move.to) + ".");
	}
}

void Game::_validateBishopMove(const Move& move) const {
	int diffRank = abs((int)move.to.rank - (int)move.from.rank), diffFile = abs((int)move.to.file - (int)move.from.file);

	if (diffRank != diffFile) {
		throw runtime_error("Illegal bishop move from " + to_string(move.from) + " to " + to_string(move.to) + ".");
	}

	int rankDir = ((int)move.to.rank - (int)move.from.rank) < 0 ? -1 : 1, fileDir = ((int)move.to.file - (int)move.from.file) < 0 ? -1 : 1;

	for (uint i = 1; i < (uint)diffRank; i++) {
		uint file = move.from.file + fileDir * i, rank = move.from.rank + rankDir * i;
		Position pos = {.file = (Files)file, .rank = rank};

		if (hasPiece(pos)) {
			throw runtime_error("Illegal bishop move from " + to_string(move.from) + " to " + to_string(move.to) + ": intervening piece on " +
								to_string(pos) + ".");
		}
	}
}

void Game::_validateRookMove(const Move& move) const {
	if (move.to.file != move.from.file && move.to.rank != move.from.rank) {
		throw runtime_error("Illegal rook move from " + to_string(move.from) + " to " + to_string(move.to) + ".");
	}

	if (move.to.file != move.from.file) {
		int fileDir = ((int)move.to.file - (int)move.from.file) < 0 ? -1 : 1;

		for (uint file = move.from.file + fileDir; file != move.to.file; file += fileDir) {
			Position pos = {.file = (Files)file, .rank = move.from.rank};

			if (hasPiece(pos)) {
				throw runtime_error("Illegal rook move from " + to_string(move.from) + " to " + to_string(move.to) + ": intervening piece on " +
									to_string(pos) + ".");
			}
		}
	} else {
		int rankDir = ((int)move.to.rank - (int)move.from.rank) < 0 ? -1 : 1;

		for (uint rank = move.from.rank + rankDir; rank != move.to.rank; rank += rankDir) {
			Position pos = {.file = move.from.file, .rank = rank};

			if (hasPiece(pos)) {
				throw runtime_error("Illegal rook move from " + to_string(move.from) + " to " + to_string(move.to) + ": intervening piece on " +
									to_string(pos) + ".");
			}
		}
	}
}

void Game::_validateKingMove(const Move& move) const {
	// TODO: may have to fix check logic by disallowing pieces from moving to their own square in each piece's logic
	int diffRank = abs((int)move.to.rank - (int)move.from.rank), diffFile = abs((int)move.to.file - (int)move.from.file);

	if (diffRank > 1) {
		throw runtime_error("Illegal king move from " + to_string(move.from) + " to " + to_string(move.to) + ".");
	}

	if (diffFile > 1) {
		// consider castling
		int castleDir = (int)move.to.file - (int)move.from.file < 0 ? -1 : 1;
		Piece king = getPiece(move.from), rook = getPiece({.file = castleDir == -1 ? Files::A : Files::H, .rank = _turn == Players::WHITE ? 1u : 8u});

		if (diffFile == 2 && !king._moved && !rook._moved) {
			const vector<Piece>& other = _turn == Players::WHITE ? _black : _white;

			for (const Piece& opponentPiece : other) {
				switch (opponentPiece._type) {
					case PieceTypes::PAWN:
						try {
							// TODO: no idea if en passant is an edge case here? (i think it shouldnt)
							_validatePawnMove({.from = opponentPiece._position, .to = move.from});
							break;	// breaks out of switch block, triggering "trap"
						} catch (...) {
							continue;  // skips the "trap" at the bottom of each iteration
						}
					case PieceTypes::KNIGHT:
						try {
							_validateKnightMove({.from = opponentPiece._position, .to = move.from});
							break;
						} catch (...) {
							continue;
						}
					case PieceTypes::BISHOP:
						try {
							_validateBishopMove({.from = opponentPiece._position, .to = move.from});
							break;
						} catch (...) {
							continue;
						}
					case PieceTypes::ROOK:
						try {
							_validateRookMove({.from = opponentPiece._position, .to = move.from});
							break;
						} catch (...) {
							continue;
						}
					case PieceTypes::QUEEN:
						try {
							_validateRookMove({.from = opponentPiece._position, .to = move.from});
							break;
						} catch (...) {
							try {
								_validateBishopMove({.from = opponentPiece._position, .to = move.from});
								break;
							} catch (...) {
								continue;
							}
						}
					case PieceTypes::KING:
						try {
							_validateKingMove({.from = opponentPiece._position, .to = move.from});
							break;
						} catch (...) {
							continue;
						}
				}

				throw runtime_error("Illegal king move: castling out of check.");
			}
		} else {
			throw runtime_error("Illegal king move from " + to_string(move.from) + " to " + to_string(move.to) + ".");
		}
	}
}