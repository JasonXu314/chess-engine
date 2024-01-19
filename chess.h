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

	char symbol() const;

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
	Game(const Game& other);

	// returns true if pawn reached promotion (also sets shouldPromote private variable)
	bool move(const Move& move);

	void promote(const Position& pos, PieceTypes to);

	bool isChecked() const;
	bool isChecked(Players player) const;

	Piece getPiece(const Position& pos) const;

	bool hasPiece(const Position& pos) const;

	const Board& board() const;

	Players turn() const;

	bool shouldPromote() const;

private:
	Board _board;
	std::vector<Piece> _white;
	std::vector<Piece> _black;
	Players _turn;
	bool _firstMove;
	Move _prevMove;
	bool _shouldPromote;

	Piece& _getPieceRef(const Position& pos);

	// Not a direct substitute for the public version, is needed for timing reliability (en passant edge case in move), and is less performant
	bool _hasPiece(const Position& pos) const;

	void _validatePawnMove(const Move& move) const;
	void _validateKnightMove(const Move& move) const;
	void _validateBishopMove(const Move& move) const;
	void _validateRookMove(const Move& move) const;
	void _validateKingMove(const Move& move) const;
};

#endif