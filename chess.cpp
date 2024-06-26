#include "chess.h"

using namespace std;

int TWOS[2] = {-2, 2}, ONES[2] = {-1, 1};

bool operator==(const Position& a, const Position& b) {
	return a.file == b.file && a.rank == b.rank;
}

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

const Position& Piece::position() const {
	return _position;
}
PieceTypes Piece::type() const {
	return _type;
}
Players Piece::player() const {
	return _player;
}
char Piece::symbol() const {
	return _symbol;
}

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

Game::Game() : _turn(Players::WHITE), _firstMove(true), _prevMoveEnPassant(false), _shouldPromote(false), _turns(0), _halfTurnsSinceCapture(0) {
	for (const Files file : FILES) {
		_white.push_back(Piece('P', {.file = file, .rank = 2}));
		_black.push_back(Piece('p', {.file = file, .rank = 7}));
	}

	_white.push_back(Piece('R', {.file = Files::A, .rank = 1}));
	_white.push_back(Piece('R', {.file = Files::H, .rank = 1}));
	_white.push_back(Piece('N', {.file = Files::B, .rank = 1}));
	_white.push_back(Piece('N', {.file = Files::G, .rank = 1}));
	_white.push_back(Piece('B', {.file = Files::C, .rank = 1}));
	_white.push_back(Piece('B', {.file = Files::F, .rank = 1}));
	_white.push_back(Piece('Q', {.file = Files::D, .rank = 1}));
	_white.push_back(Piece('K', {.file = Files::E, .rank = 1}));

	_black.push_back(Piece('r', {.file = Files::A, .rank = 8}));
	_black.push_back(Piece('r', {.file = Files::H, .rank = 8}));
	_black.push_back(Piece('n', {.file = Files::B, .rank = 8}));
	_black.push_back(Piece('n', {.file = Files::G, .rank = 8}));
	_black.push_back(Piece('b', {.file = Files::C, .rank = 8}));
	_black.push_back(Piece('b', {.file = Files::F, .rank = 8}));
	_black.push_back(Piece('q', {.file = Files::D, .rank = 8}));
	_black.push_back(Piece('k', {.file = Files::E, .rank = 8}));
}

Game::Game(const Game& other)
	: _turn(other._turn),
	  _firstMove(other._firstMove),
	  _prevMove(other._prevMove),
	  _prevMoveEnPassant(other._prevMoveEnPassant),
	  _shouldPromote(other._shouldPromote) {
	for (const Piece& piece : other._white) {
		_white.push_back(Piece(piece._symbol, piece._position));
	}
	for (const Piece& piece : other._black) {
		_black.push_back(Piece(piece._symbol, piece._position));
	}
}

Game::Game(const string& fen) : _turn(Players::WHITE), _firstMove(true), _prevMoveEnPassant(false), _shouldPromote(false) {
	uint i = 0;

	Position currPos = {.file = Files::A, .rank = 8};
	while (fen[i] != ' ') {
		if (isdigit(fen[i])) {
			uint numSpaces = fen[i] - '0';

			currPos.file = (Files)((int)currPos.file + numSpaces);
		} else {
			if (fen[i] == '/') {
				currPos.file = Files::A;
				currPos.rank--;
			} else {
				Piece piece(fen[i], currPos);
				if (piece._type == PieceTypes::ROOK) {
					piece._moved = true;  // prepare for later
				}

				if (piece.player() == Players::WHITE) {
					_white.push_back(piece);
				} else {
					_black.push_back(piece);
				}

				currPos.file = (Files)((int)currPos.file + 1);
			}
		}

		i++;
	}

	i++;

	if (fen[i] == 'b') {
		_turn = Players::BLACK;
	}

	i += 2;

	if (fen[i] != '-') {
		while (fen[i] != ' ') {
			if (isupper(fen[i])) {
				if (fen[i] == 'K' && hasPiece({.file = Files::H, .rank = 1})) {
					Piece& rook = _getPieceRef({.file = Files::H, .rank = 1});

					if (rook._type == PieceTypes::ROOK) {
						rook._moved = false;
					}
				}
				if (fen[i] == 'Q' && hasPiece({.file = Files::A, .rank = 1})) {
					Piece& rook = _getPieceRef({.file = Files::A, .rank = 1});

					if (rook._type == PieceTypes::ROOK) {
						rook._moved = false;
					}
				}
			} else {
				if (fen[i] == 'k' && hasPiece({.file = Files::H, .rank = 8})) {
					Piece& rook = _getPieceRef({.file = Files::H, .rank = 8});

					if (rook._type == PieceTypes::ROOK) {
						rook._moved = false;
					}
				}
				if (fen[i] == 'q' && hasPiece({.file = Files::A, .rank = 8})) {
					Piece& rook = _getPieceRef({.file = Files::A, .rank = 8});

					if (rook._type == PieceTypes::ROOK) {
						rook._moved = false;
					}
				}
			}

			i++;
		}
	}

	i++;
	if (fen[i] != ' ') {
		_prevMoveEnPassant = true;
	}

	i++;
	string halfMoveClock;
	while (fen[i] != ' ') {
		halfMoveClock += fen[i];
		i++;
	}

	_halfTurnsSinceCapture = stoi(halfMoveClock);

	i++;
	string fullMoveClock;
	while (fen[i] != ' ') {
		fullMoveClock += fen[i];
		i++;
	}

	_turns = stoi(fullMoveClock);
}

bool Game::move(const Move& move) {
	if (_shouldPromote) {
		throw runtime_error("Select a promotion piece first.");
	}

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

	if (_uncheckedBranch(move).isChecked()) {
		throw runtime_error("Illegal move: moving into check/moving while in check.");
	}

	_prevMoveEnPassant = false;
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
		_halfTurnsSinceCapture = 0;
	} else if (piece._type == PieceTypes::PAWN && move.to.file != move.from.file && !hasPiece(move.to) &&
			   hasPiece({.file = move.to.file, .rank = move.to.rank + (_turn == Players::WHITE ? -1 : 1)})) {
		uint capturedIdx = (uint)-1;
		for (uint i = 0; i < other.size(); i++) {
			if (other[i]._position == Position{.file = move.to.file, .rank = move.to.rank + (_turn == Players::WHITE ? -1 : 1)} &&
				other[i]._type == PieceTypes::PAWN) {
				capturedIdx = i;
			}
		}

		if (capturedIdx != (uint)-1) {
			other.erase(other.begin() + capturedIdx);
		} else {
			throw runtime_error("Shit done fucked up (capture logic)");
		}

		_prevMoveEnPassant = true;
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
	}

	_prevMove = move;

	if (_firstMove) {
		_firstMove = false;
	}
	if (!piece._moved) {
		piece._moved = true;
	}

	if (_turn == Players::BLACK) {
		_turns++;
	}

	if (!isCapture) {
		_halfTurnsSinceCapture++;
	}

	if (piece._type == PieceTypes::PAWN && move.to.rank == (_turn == Players::WHITE ? 8 : 1)) {
		_shouldPromote = true;
		return true;
	} else {
		_turn = (_turn == Players::WHITE ? Players::BLACK : Players::WHITE);

		return false;
	}
}

Game Game::branch(const Move& move) const {
	Game future(*this);

	try {
		future.move(move);

		return future;
	} catch (const runtime_error& err) {
		throw runtime_error("Branching error: move failed with '" + string(err.what()) + "'");
	} catch (...) {
		throw runtime_error("Branching error: move failed with unknown error.");
	}
}

Game Game::_uncheckedBranch(const Move& move) const {
	Game future(*this);

	try {
		Piece& piece = future._getPieceRef(move.from);
		piece._position = move.to;

		return future;
	} catch (const runtime_error& err) {
		throw runtime_error("Branching error: move failed with '" + string(err.what()) + "'");
	} catch (...) {
		throw runtime_error("Branching error: move failed with unknown error.");
	}
}

Game Game::branchPromote(const Position& pos, PieceTypes to) const {
	Game future(*this);

	try {
		future.promote(pos, to);

		return future;
	} catch (const runtime_error& err) {
		throw runtime_error("Branching error: promotion failed with '" + string(err.what()) + "'");
	} catch (...) {
		throw runtime_error("Branching error: promotion failed with unknown error.");
	}
}

vector<Move> Game::getAvailableMoves() const {
	vector<Move> out;

	const vector<Piece>& player = _turn == Players::WHITE ? _white : _black;

	// first, naively generate available moves based only on piece location
	for (const Piece& piece : player) {
		switch (piece._type) {
			case PieceTypes::PAWN:
				if (piece._position.rank == (_turn == Players::WHITE ? 2 : 7) &&
					!hasPiece({.file = piece._position.file, .rank = piece._position.rank + (_turn == Players::WHITE ? 2 : -2)})) {
					out.push_back(
						{.from = piece._position, .to = {.file = piece._position.file, .rank = piece._position.rank + (_turn == Players::WHITE ? 2 : -2)}});
				}
				if (!hasPiece({.file = piece._position.file, .rank = piece._position.rank + (_turn == Players::WHITE ? 1 : -1)})) {
					out.push_back(
						{.from = piece._position, .to = {.file = piece._position.file, .rank = piece._position.rank + (_turn == Players::WHITE ? 1 : -1)}});
				}

				if (piece._position.file != Files::A) {
					Position captureLeft = {.file = (Files)((int)piece._position.file - 1),
											.rank = piece._position.rank + (_turn == Players::WHITE ? 1 : -1)};

					if (hasPiece(captureLeft) && getPiece(captureLeft)._player != _turn) {
						out.push_back({.from = piece._position, .to = captureLeft});
					}
				}
				if (piece._position.file != Files::H) {
					Position captureRight = {.file = (Files)((int)piece._position.file + 1),
											 .rank = piece._position.rank + (_turn == Players::WHITE ? 1 : -1)};

					if (hasPiece(captureRight) && getPiece(captureRight)._player != _turn) {
						out.push_back({.from = piece._position, .to = captureRight});
					}
				}
				break;
			case PieceTypes::KNIGHT:
				for (int fDelta : TWOS) {
					for (int rDelta : ONES) {
						int newFile = (int)piece._position.file + fDelta, newRank = piece._position.rank + rDelta;

						if (newFile >= 0 && newFile < 8 && newRank >= 1 && newRank <= 8) {
							Position newPos = {.file = (Files)newFile, .rank = (uint)newRank};

							if (!hasPiece(newPos) || (hasPiece(newPos) && getPiece(newPos)._player != _turn)) {
								out.push_back({.from = piece._position, .to = newPos});
							}
						}
					}
				}

				for (int fDelta : ONES) {
					for (int rDelta : TWOS) {
						int newFile = (int)piece._position.file + fDelta, newRank = piece._position.rank + rDelta;

						if (newFile >= 0 && newFile < 8 && newRank >= 1 && newRank <= 8) {
							Position newPos = {.file = (Files)newFile, .rank = (uint)newRank};

							if (!hasPiece(newPos) || (hasPiece(newPos) && getPiece(newPos)._player != _turn)) {
								out.push_back({.from = piece._position, .to = newPos});
							}
						}
					}
				}
				break;
			case PieceTypes::BISHOP:
				// could probably optimize by checking in all 4 directions at once, but would require some ugly bool stuff i think
				for (uint delta = 1; delta < 8; delta++) {
					int newFile = (int)piece._position.file + delta, newRank = piece._position.rank + delta;

					if (newFile >= 0 && newFile < 8 && newRank >= 1 && newRank <= 8) {
						Position newPos = {.file = (Files)newFile, .rank = (uint)newRank};

						if (!hasPiece(newPos) || (hasPiece(newPos) && getPiece(newPos)._player != _turn)) {
							out.push_back({.from = piece._position, .to = newPos});
						}
					} else {
						break;
					}
				}
				for (uint delta = 1; delta < 8; delta++) {
					int newFile = (int)piece._position.file + delta, newRank = piece._position.rank - delta;

					if (newFile >= 0 && newFile < 8 && newRank >= 1 && newRank <= 8) {
						Position newPos = {.file = (Files)newFile, .rank = (uint)newRank};

						if (!hasPiece(newPos) || (hasPiece(newPos) && getPiece(newPos)._player != _turn)) {
							out.push_back({.from = piece._position, .to = newPos});
						}
					} else {
						break;
					}
				}
				for (uint delta = 1; delta < 8; delta++) {
					int newFile = (int)piece._position.file - delta, newRank = piece._position.rank + delta;

					if (newFile >= 0 && newFile < 8 && newRank >= 1 && newRank <= 8) {
						Position newPos = {.file = (Files)newFile, .rank = (uint)newRank};

						if (!hasPiece(newPos) || (hasPiece(newPos) && getPiece(newPos)._player != _turn)) {
							out.push_back({.from = piece._position, .to = newPos});
						}
					} else {
						break;
					}
				}
				for (uint delta = 1; delta < 8; delta++) {
					int newFile = (int)piece._position.file - delta, newRank = piece._position.rank - delta;

					if (newFile >= 0 && newFile < 8 && newRank >= 1 && newRank <= 8) {
						Position newPos = {.file = (Files)newFile, .rank = (uint)newRank};

						if (!hasPiece(newPos) || (hasPiece(newPos) && getPiece(newPos)._player != _turn)) {
							out.push_back({.from = piece._position, .to = newPos});
						}
					} else {
						break;
					}
				}
				break;
			case PieceTypes::ROOK:
				// could probably optimize by checking in all 4 directions at once, but would require some ugly bool stuff i think
				for (uint delta = 1; delta < 8; delta++) {
					int newFile = (int)piece._position.file + delta, newRank = piece._position.rank;

					if (newFile >= 0 && newFile < 8 && newRank >= 1 && newRank <= 8) {
						Position newPos = {.file = (Files)newFile, .rank = (uint)newRank};

						if (!hasPiece(newPos) || (hasPiece(newPos) && getPiece(newPos)._player != _turn)) {
							out.push_back({.from = piece._position, .to = newPos});
						}
					} else {
						break;
					}
				}
				for (uint delta = 1; delta < 8; delta++) {
					int newFile = (int)piece._position.file - delta, newRank = piece._position.rank;

					if (newFile >= 0 && newFile < 8 && newRank >= 1 && newRank <= 8) {
						Position newPos = {.file = (Files)newFile, .rank = (uint)newRank};

						if (!hasPiece(newPos) || (hasPiece(newPos) && getPiece(newPos)._player != _turn)) {
							out.push_back({.from = piece._position, .to = newPos});
						}
					} else {
						break;
					}
				}
				for (uint delta = 1; delta < 8; delta++) {
					int newFile = (int)piece._position.file, newRank = piece._position.rank + delta;

					if (newFile >= 0 && newFile < 8 && newRank >= 1 && newRank <= 8) {
						Position newPos = {.file = (Files)newFile, .rank = (uint)newRank};

						if (!hasPiece(newPos) || (hasPiece(newPos) && getPiece(newPos)._player != _turn)) {
							out.push_back({.from = piece._position, .to = newPos});
						}
					} else {
						break;
					}
				}
				for (uint delta = 1; delta < 8; delta++) {
					int newFile = (int)piece._position.file, newRank = piece._position.rank - delta;

					if (newFile >= 0 && newFile < 8 && newRank >= 1 && newRank <= 8) {
						Position newPos = {.file = (Files)newFile, .rank = (uint)newRank};

						if (!hasPiece(newPos) || (hasPiece(newPos) && getPiece(newPos)._player != _turn)) {
							out.push_back({.from = piece._position, .to = newPos});
						}
					} else {
						break;
					}
				}
				break;
			case PieceTypes::QUEEN:
				for (uint delta = 1; delta < 8; delta++) {
					int newFile = (int)piece._position.file + delta, newRank = piece._position.rank + delta;

					if (newFile >= 0 && newFile < 8 && newRank >= 1 && newRank <= 8) {
						Position newPos = {.file = (Files)newFile, .rank = (uint)newRank};

						if (!hasPiece(newPos) || (hasPiece(newPos) && getPiece(newPos)._player != _turn)) {
							out.push_back({.from = piece._position, .to = newPos});
						}
					} else {
						break;
					}
				}
				for (uint delta = 1; delta < 8; delta++) {
					int newFile = (int)piece._position.file + delta, newRank = piece._position.rank - delta;

					if (newFile >= 0 && newFile < 8 && newRank >= 1 && newRank <= 8) {
						Position newPos = {.file = (Files)newFile, .rank = (uint)newRank};

						if (!hasPiece(newPos) || (hasPiece(newPos) && getPiece(newPos)._player != _turn)) {
							out.push_back({.from = piece._position, .to = newPos});
						}
					} else {
						break;
					}
				}
				for (uint delta = 1; delta < 8; delta++) {
					int newFile = (int)piece._position.file - delta, newRank = piece._position.rank + delta;

					if (newFile >= 0 && newFile < 8 && newRank >= 1 && newRank <= 8) {
						Position newPos = {.file = (Files)newFile, .rank = (uint)newRank};

						if (!hasPiece(newPos) || (hasPiece(newPos) && getPiece(newPos)._player != _turn)) {
							out.push_back({.from = piece._position, .to = newPos});
						}
					} else {
						break;
					}
				}
				for (uint delta = 1; delta < 8; delta++) {
					int newFile = (int)piece._position.file - delta, newRank = piece._position.rank - delta;

					if (newFile >= 0 && newFile < 8 && newRank >= 1 && newRank <= 8) {
						Position newPos = {.file = (Files)newFile, .rank = (uint)newRank};

						if (!hasPiece(newPos) || (hasPiece(newPos) && getPiece(newPos)._player != _turn)) {
							out.push_back({.from = piece._position, .to = newPos});
						}
					} else {
						break;
					}
				}
				for (uint delta = 1; delta < 8; delta++) {
					int newFile = (int)piece._position.file + delta, newRank = piece._position.rank;

					if (newFile >= 0 && newFile < 8 && newRank >= 1 && newRank <= 8) {
						Position newPos = {.file = (Files)newFile, .rank = (uint)newRank};

						if (!hasPiece(newPos) || (hasPiece(newPos) && getPiece(newPos)._player != _turn)) {
							out.push_back({.from = piece._position, .to = newPos});
						}
					} else {
						break;
					}
				}
				for (uint delta = 1; delta < 8; delta++) {
					int newFile = (int)piece._position.file - delta, newRank = piece._position.rank;

					if (newFile >= 0 && newFile < 8 && newRank >= 1 && newRank <= 8) {
						Position newPos = {.file = (Files)newFile, .rank = (uint)newRank};

						if (!hasPiece(newPos) || (hasPiece(newPos) && getPiece(newPos)._player != _turn)) {
							out.push_back({.from = piece._position, .to = newPos});
						}
					} else {
						break;
					}
				}
				for (uint delta = 1; delta < 8; delta++) {
					int newFile = (int)piece._position.file, newRank = piece._position.rank + delta;

					if (newFile >= 0 && newFile < 8 && newRank >= 1 && newRank <= 8) {
						Position newPos = {.file = (Files)newFile, .rank = (uint)newRank};

						if (!hasPiece(newPos) || (hasPiece(newPos) && getPiece(newPos)._player != _turn)) {
							out.push_back({.from = piece._position, .to = newPos});
						}
					} else {
						break;
					}
				}
				for (uint delta = 1; delta < 8; delta++) {
					int newFile = (int)piece._position.file, newRank = piece._position.rank - delta;

					if (newFile >= 0 && newFile < 8 && newRank >= 1 && newRank <= 8) {
						Position newPos = {.file = (Files)newFile, .rank = (uint)newRank};

						if (!hasPiece(newPos) || (hasPiece(newPos) && getPiece(newPos)._player != _turn)) {
							out.push_back({.from = piece._position, .to = newPos});
						}
					} else {
						break;
					}
				}
				break;
			case PieceTypes::KING:
				// start from top left and search clockwise
				Position newPos = {.file = (Files)((int)piece._position.file - 1), .rank = piece._position.rank + 1};

				if (newPos.file >= 0 && newPos.file < 8 && newPos.rank >= 1 && newPos.rank <= 8 &&
					(!hasPiece(newPos) || (hasPiece(newPos) && getPiece(newPos)._player != _turn))) {
					out.push_back({.from = piece._position, .to = newPos});
				}

				newPos.file = (Files)((int)newPos.file + 1);

				if (newPos.file >= 0 && newPos.file < 8 && newPos.rank >= 1 && newPos.rank <= 8 &&
					(!hasPiece(newPos) || (hasPiece(newPos) && getPiece(newPos)._player != _turn))) {
					out.push_back({.from = piece._position, .to = newPos});
				}

				newPos.file = (Files)((int)newPos.file + 1);

				if (newPos.file >= 0 && newPos.file < 8 && newPos.rank >= 1 && newPos.rank <= 8 &&
					(!hasPiece(newPos) || (hasPiece(newPos) && getPiece(newPos)._player != _turn))) {
					out.push_back({.from = piece._position, .to = newPos});
				}

				newPos.rank--;

				if (newPos.file >= 0 && newPos.file < 8 && newPos.rank >= 1 && newPos.rank <= 8 &&
					(!hasPiece(newPos) || (hasPiece(newPos) && getPiece(newPos)._player != _turn))) {
					out.push_back({.from = piece._position, .to = newPos});
				}

				newPos.rank--;

				if (newPos.file >= 0 && newPos.file < 8 && newPos.rank >= 1 && newPos.rank <= 8 &&
					(!hasPiece(newPos) || (hasPiece(newPos) && getPiece(newPos)._player != _turn))) {
					out.push_back({.from = piece._position, .to = newPos});
				}

				newPos.file = (Files)((int)newPos.file - 1);

				if (newPos.file >= 0 && newPos.file < 8 && newPos.rank >= 1 && newPos.rank <= 8 &&
					(!hasPiece(newPos) || (hasPiece(newPos) && getPiece(newPos)._player != _turn))) {
					out.push_back({.from = piece._position, .to = newPos});
				}

				newPos.file = (Files)((int)newPos.file - 1);

				if (newPos.file >= 0 && newPos.file < 8 && newPos.rank >= 1 && newPos.rank <= 8 &&
					(!hasPiece(newPos) || (hasPiece(newPos) && getPiece(newPos)._player != _turn))) {
					out.push_back({.from = piece._position, .to = newPos});
				}

				newPos.rank++;

				if (newPos.file >= 0 && newPos.file < 8 && newPos.rank >= 1 && newPos.rank <= 8 &&
					(!hasPiece(newPos) || (hasPiece(newPos) && getPiece(newPos)._player != _turn))) {
					out.push_back({.from = piece._position, .to = newPos});
				}

				// account for castling
				if (!piece._moved) {
					if (hasPiece({.file = Files::H, .rank = _turn == Players::WHITE ? 1u : 8u})) {
						Piece kingRook = getPiece({.file = Files::H, .rank = _turn == Players::WHITE ? 1u : 8u});

						if (!kingRook._moved) {
							out.push_back({.from = piece._position, .to = {.file = Files::G, .rank = _turn == Players::WHITE ? 1u : 8u}});
						}
					}
					if (hasPiece({.file = Files::A, .rank = _turn == Players::WHITE ? 1u : 8u})) {
						Piece queenRook = getPiece({.file = Files::A, .rank = _turn == Players::WHITE ? 1u : 8u});

						if (!queenRook._moved) {
							out.push_back({.from = piece._position, .to = {.file = Files::C, .rank = _turn == Players::WHITE ? 1u : 8u}});
						}
					}
				}
				break;
		}
	}

	// then, validate with futures (not with move because that would modify things if a move succeeds)
	for (uint i = 0; i < out.size(); i++) {
		try {
			branch(out[i]);
		} catch (...) {
			out.erase(out.begin() + i);
			i--;
		}
	}

	return out;
}

uint Game::materiel(Players player) const {
	uint total = 0;

	for (const Piece& piece : player == Players::WHITE ? _white : _black) {
		total += valueOf(piece._type);
	}

	return total;
}

void Game::promote(const Position& pos, PieceTypes to) {
	if (!_shouldPromote) {
		throw runtime_error("No promotion available.");
	}

	Piece& piece = _getPieceRef(pos);

	if (piece._player != _turn) {
		throw runtime_error("Promotion piece on " + to_string(pos) + " does not belong to moving player.");
	}
	if (piece._type != PieceTypes::PAWN) {
		throw runtime_error("Promotion piece " + to_string(pos) + " must be a pawn.");
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

	_turn = (_turn == Players::WHITE ? Players::BLACK : Players::WHITE);
	_shouldPromote = false;
}

Piece Game::getPiece(const Position& pos) const {
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

	throw runtime_error("No piece at position " + to_string(pos));
}

bool Game::isChecked() const {
	return isChecked(_turn);
}

bool Game::isChecked(Players player) const {
	const vector<Piece>&thisPlayer = player == Players::WHITE ? _white : _black, &other = player == Players::WHITE ? _black : _white;
	Position kingPos;

	for (const Piece& piece : thisPlayer) {
		if (piece._type == PieceTypes::KING) {
			kingPos = piece._position;
		}
	}

	for (const Piece& opponentPiece : other) {
		switch (opponentPiece._type) {
			case PieceTypes::PAWN:
				try {
					// TODO: no idea if en passant is an edge case here? (i think it shouldnt)
					_validatePawnMove({.from = opponentPiece._position, .to = kingPos});
					break;	// normal breakout of switch block, triggering "trap"
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

		return true;
	}

	return false;
}

string Game::dumpFEN() const {
	string fen;

	for (uint rank = 8; rank >= 1; rank--) {
		for (Files file : FILES) {
			if (hasPiece({.file = file, .rank = rank})) {
				fen += getPiece({.file = file, .rank = rank})._symbol;
			} else {
				if (fen.back() >= '0' && fen.back() <= '9') {
					fen.back()++;
				} else {
					fen += '1';
				}
			}
		}

		fen += "/";
	}

	fen.pop_back();
	fen += ' ';
	fen += _turn == Players::WHITE ? 'w' : 'b';

	fen += ' ';
	bool wKingCastle = false;
	for (const Piece& piece : _white) {
		if (piece._type == PieceTypes::KING) {
			wKingCastle = !piece._moved;
			break;
		}
	}

	if (wKingCastle) {
		for (const Piece& piece : _white) {
			if (piece._type == PieceTypes::ROOK && !piece._moved) {
				if (piece._position == Position{.file = Files::A, .rank = 1}) {
					fen += 'K';
				} else if (piece._position == Position{.file = Files::H, .rank = 1}) {
					fen += 'Q';
				} else {
					throw runtime_error("Piece state done fucked up");
				}
			}
		}
	}

	bool bKingCastle = false;
	for (const Piece& piece : _black) {
		if (piece._type == PieceTypes::KING) {
			bKingCastle = !piece._moved;
			break;
		}
	}

	if (bKingCastle) {
		for (const Piece& piece : _black) {
			if (piece._type == PieceTypes::ROOK && !piece._moved) {
				if (piece._position == Position{.file = Files::A, .rank = 8}) {
					fen += 'k';
				} else if (piece._position == Position{.file = Files::H, .rank = 8}) {
					fen += 'q';
				} else {
					throw runtime_error("Piece state done fucked up");
				}
			}
		}
	}

	if (fen.back() == ' ') {
		fen += "- ";
	}

	if (_prevMoveEnPassant) {
		string moveStr = to_string(_prevMove.to);
		moveStr[0] = tolower(moveStr[0]);

		fen += moveStr;
	} else {
		fen += '-';
	}

	fen += ' ';
	fen += to_string(_halfTurnsSinceCapture);

	fen += ' ';
	fen += to_string(_turns);

	return fen;
}

Piece& Game::_getPieceRef(const Position& pos) {
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

	throw runtime_error("No piece at position " + to_string(pos));
}

bool Game::hasPiece(const Position& pos) const {
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

Players Game::turn() const {
	return _turn;
}
bool Game::shouldPromote() const {
	return _shouldPromote;
}

Game& Game::operator=(const Game& other) {
	_turn = other._turn;
	_firstMove = other._firstMove;
	_prevMove = other._prevMove;
	_shouldPromote = other._shouldPromote;

	for (const Piece& piece : other._white) {
		_white.push_back(Piece(piece._symbol, piece._position));
	}
	for (const Piece& piece : other._black) {
		_black.push_back(Piece(piece._symbol, piece._position));
	}

	return *this;
}

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

		if (diffFile == 2 && king._type == PieceTypes::KING && !king._moved && rook._type == PieceTypes::ROOK && !rook._moved) {
			for (uint file = king._position.file + castleDir; file < rook._position.file; file += castleDir) {
				if (hasPiece({.file = (Files)file, .rank = _turn == Players::WHITE ? 1u : 8u})) {
					throw runtime_error("Illegal king move: castling through piece.");
				}
			}

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