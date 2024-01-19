#ifndef CHESS_H
#define CHESS_H

#include <iostream>
#include <string>
#include <vector>

#include "board.h"

struct Position {
	Files file;
	uint rank;
};

bool operator==(const Position& a, const Position& b);

std::string to_string(const Position& pos);

struct Move {
	Position from;
	Position to;
};

class Piece {
public:
	Piece(char symbol, Position position);

	const Position& position() const;

	PieceTypes type() const;

	Players player() const;

	friend class Game;
	friend std::ostream& operator<<(std::ostream& out, const Piece& piece);

private:
	Position _position;
	PieceTypes _type;
	uint _value;
	Players _player;
	char _symbol;
	bool _moved;
};

std::ostream& operator<<(std::ostream& out, const Piece& piece);

class Game {
public:
	Game();

	void move(const Move& move);

	Piece getPiece(const Position& pos) const;

	bool hasPiece(const Position& pos) const;

private:
	Board _board;
	std::vector<Piece> _white;
	std::vector<Piece> _black;
	Players _turn;
	bool _firstMove;
	Move _prevMove;

	Piece& _getPieceRef(const Position& pos);

	void _validatePawnMove(const Move& move) const;
	void _validateKnightMove(const Move& move) const;
	void _validateBishopMove(const Move& move) const;
	void _validateRookMove(const Move& move) const;
	void _validateKingMove(const Move& move) const;
};

#endif