#include "board.h"

using namespace std;

Board::Board() : _board(new char[64]) {
	for (int i = 0; i < 64; i++) {
		_board[i] = '\0';
	}
}

Board::Board(const Board& other) : _board(new char[64]) {
	for (int i = 0; i < 64; i++) {
		_board[i] = other._board[i];
	}
}

char Board::get(uint rank, uint file) const {
	if (rank > 7 || file > 7) throw runtime_error("Out of bounds.");

	return _board[file * 8 + rank];
}

Board::File Board::operator[](Files file) const { return File(file, _board); }

Board::~Board() { delete[] _board; }

char& Board::File::operator[](uint rank) const {
	_ensureRange(rank);

	return _board[_file * 8 + rank - 1];
}
char& Board::File::operator[](uint rank) {
	_ensureRange(rank);

	return _board[_file * 8 + rank - 1];
}

Board& Board::operator=(const Board& other) {
	for (int i = 0; i < 64; i++) {
		_board[i] = other._board[i];
	}

	return *this;
}

void Board::File::_ensureRange(uint rank) const {
	if (rank > 8 || rank < 1) {
		throw runtime_error("Rank " + to_string(rank) + " out of bounds.");
	}
}