#ifndef BOARD_H
#define BOARD_H

#include <stdexcept>

#include "constants.h"
#include "types.h"

class Board {
public:
	Board();
	Board(const Board& other);

	char get(uint rank, uint file) const;

	class File {
	public:
		File(Files file, char* board) noexcept : _file(file), _board(board) {}
		File(const File& other) noexcept : _file(other._file), _board(other._board) {}

		char& operator[](uint rank) const;
		char& operator[](uint rank);

	private:
		Files _file;
		char* _board;

		void _ensureRange(uint rank) const;
	};

	File operator[](Files file) const;

	Board& operator=(const Board& other);

	~Board();

private:
	char* _board;
};

#endif